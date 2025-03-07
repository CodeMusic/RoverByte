import speech_recognition as sr
from openai_helper import OpenAIHelper
from api_server import OpenAIHelperServer  
from config import OPENAI_API_KEY, OPENAI_ASSISTANT_ID, TTS_VOICE, VOLUME_DB, LANGUAGE
from utils import sox_volume, grey_print, debug_print
import time
import re
import ast
import os
import base64

"""
AIInteraction is the main class to interact with the dog.
It provides a layer of abstraction over the OpenAI API, and the speech recognition library.
"""

class AIInteraction:
    def __init__(self, assistant_config: dict = None):
        # Default to RoverByte if no config provided
        if assistant_config is None:
            assistant_config = {
                'name': 'RoverByte',
                'api_key': OPENAI_API_KEY,
                'assistant_id': OPENAI_ASSISTANT_ID,
                'voice': TTS_VOICE,
                'language': LANGUAGE
            }
        
        self.assistant_config = assistant_config
        self.openai_helper = OpenAIHelper(
            api_key=self.assistant_config['api_key'],
            assistant_id=self.assistant_config['assistant_id'],
            assistant_name=self.assistant_config['name']
        )
        self.recognizer = sr.Recognizer()
        self.setup_recognizer()
        self.server = None


    def start_server(self, dog_controller=None):
        """Start the API server without blocking sensor handlers"""
        try:
            debug_print("Initializing API server...")
            if not hasattr(self, 'server') or self.server is None:
                self.server = OpenAIHelperServer(self.openai_helper, dog_controller, self)
                self.server.start()
                debug_print("API server started successfully")
            else:
                debug_print("Server already running")
        except Exception as e:
            debug_print(f"Error starting server: {e}")

    def stop_server(self):
        """Stop the API server"""
        try:
            if hasattr(self, 'server') and self.server:
                self.server.stop()
                self.server = None
                debug_print("API server stopped")
        except Exception as e:
            debug_print(f"Error stopping server: {e}")

    def setup_recognizer(self):
        self.recognizer.dynamic_energy_adjustment_damping = 0.1 # Reduce damping for quicker adaptation to sound changes
        self.recognizer.dynamic_energy_ratio = 1.2 # Lower ratio to better detect short commands
        self.recognizer.energy_threshold = 50 # Lower threshold for better detection of quiet speech
        self.recognizer.dynamic_energy_threshold = True # Keep dynamic threshold adjustment for adaptability
        self.recognizer.pause_threshold = 1.0 # Increase pause threshold to prevent cutting off speech
        self.recognizer.operation_timeout = None # No timeout to allow for thoughtful responses
        self.recognizer.phrase_threshold = 0.3 # Lower phrase threshold to better catch short commands
        self.recognizer.non_speaking_duration = 0.8 # Increase duration to prevent premature cut-offs

    def listen_for_audio(self):
        with sr.Microphone(chunk_size=8192) as source:
            self.recognizer.adjust_for_ambient_noise(source)
            return self.recognizer.listen(source)

    def speech_to_text(self, audio, language):
        st = time.time()
        result = self.openai_helper.stt(audio, language=language)
        grey_print(f"stt takes: {time.time() - st:.3f} s")
        return result

    def dialogue(self, text):
        st = time.time()
        response = self.openai_helper.dialogue(text)
        grey_print(f'chat takes: {time.time() - st:.3f} s')
        return response
    
    def generate_response(self, user_input):
        try:
            response = self.openai_helper.chat_with_gpt(user_input)
            return response
        except Exception as e:
            grey_print(f"Woof! Error generating AI response: {e}")
            return "Arf! Sorry, my circuits are a bit tangled. Could you try asking again? Woof!"

    def dialogue_with_img(self, text, img_path):
        st = time.time()
        response = self.openai_helper.dialogue_with_img(text, img_path)
        grey_print(f'chat takes: {time.time() - st:.3f} s')
        return response

    def text_to_speech(self, text, voice=''):
        
        responseVoice = TTS_VOICE
        if voice != '':
            responseVoice = voice

        st = time.time()
        _time = time.strftime("%y-%m-%d_%H-%M-%S", time.localtime())
        _tts_f = f"./tts/{_time}_raw.wav"
        status = self.openai_helper.text_to_speech(text, _tts_f, responseVoice, response_format='wav')
        if status:
            tts_file = f"./tts/{_time}_{VOLUME_DB}dB.wav"
            status = sox_volume(_tts_f, tts_file, VOLUME_DB)
            grey_print(f'tts takes: {time.time() - st:.3f} s')
            return status, tts_file
        return False, None

    def parse_response(self, response): #Parse the response from the AI into speech and action sections for execution
        debug_print(f"Parsing response: {response}")
        parts = re.split(r'(\[.*?\])', response)
        result = []
        for part in parts:
            part = part.strip()
            if part.startswith('[') and part.endswith(']'):
                try:
                    actions = ast.literal_eval(part)
                    if isinstance(actions, list):
                        for action in actions:
                            result.append({"key": "action", "value": action})
                    else:
                        result.append({"key": "action", "value": actions})
                except Exception as e:
                    print(f"Error parsing action: {e}")
                    result.append({"key": "action", "value": part})
            elif part:
                result.append({"key": "content", "value": part})
        
        return result

    def generate_speech(self, text, voice=''):
        debug_print(f"Generating speech for: {text}")
        status, tts_file = self.text_to_speech(text, voice)
        return tts_file if status else None

    def clear_conversation(self):
        self.openai_helper.clear_conversation()

    def save_conversation(self, file_path):
        self.openai_helper.save_conversation(file_path)

    def load_conversation(self, file_path):
        if os.path.exists(file_path):
            self.openai_helper.load_conversation(file_path)
            return True
        return False

    def __del__(self):
        """Cleanup when object is destroyed"""
        self.stop_server()
    
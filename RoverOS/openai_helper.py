from openai import OpenAI
from utils import grey_print
from config import SAVED_OWNER_NAME 
import os
import json
from CodeMusai.TemporalLobe.Hippocampus import MemoryManager
from CodeMusai.EmotionCore import EmotionCore
from utils import chat_print
from config import SAVED_OWNER_NAME
import time
"""
This file contains the OpenAI helper class for the roverbyte.
"""


class OpenAIHelper:
    STT_OUT = "stt_output.wav"
    TTS_OUTPUT_FILE = 'tts_output.mp3'
    TIMEOUT = 30 # seconds       

    # OpenAiHelper
    # =================================================================
    def __init__(self, api_key: str, assistant_id: str, assistant_name: str, timeout=TIMEOUT) -> None:
        # Keep OpenAI client initialization with keyword args
        self.client = OpenAI(api_key=api_key, timeout=timeout)
        self.assistant_name = assistant_name
        self.assistant_id = assistant_id  # Add this line
        
        self.memory_manager = MemoryManager(self.assistant_name)
        self.owner_name = str(SAVED_OWNER_NAME)
        self.jailbreak = True
        
        # Create memory manager (just pass the string)
        print(f"DEBUG: Creating MemoryManager with assistant_name: {self.assistant_name}")
        
        
        # Create emotion core (remove keyword arguments)
        grey_print(f"DEBUG: Creating EmotionCore with assistant_name: {self.assistant_name}")
        grey_print(f"assistant_name: {self.assistant_name}")
        grey_print(f"owner_name: {self.owner_name}")
        grey_print(f"memory_manager: {self.memory_manager}")

        self.emotion_core = EmotionCore(
            self.assistant_name,
            SAVED_OWNER_NAME,
            self.memory_manager
        )

        # Context manager with keyword args is fine
        self.context_manager = AssistantContextManager(
            self.emotion_core
        )
        
        # Retrieve the assistant and its base instructions
        self.assistant = self.client.beta.assistants.retrieve(assistant_id)
        self.base_instructions = self.assistant.instructions
        
        # Initialize thread with assistant-specific config
        config_file = f'config_{self.assistant_id}.json'
        try:
            if os.path.exists(config_file):
                with open(config_file, 'r') as f:
                    config = json.load(f)
                    self.thread_id = config['thread_id']
                    try:
                        self.thread = self.client.beta.threads.retrieve(self.thread_id)
                    except Exception as e:
                        grey_print(f"Failed to retrieve thread, creating new one: {e}")
                        self.thread = self.client.beta.threads.create()
                        with open(config_file, 'w') as f:
                            json.dump({'thread_id': self.thread.id}, f)
            else:
                print("DEBUG: Thread missing, creating new one...")
                self.thread = self.client.beta.threads.create()
                thread_id = self.thread.id
                
                # Save new thread config
                config_file = f"config_{self.assistant_id}.json"
                config_data = {
                    "thread_id": thread_id,
                    "assistant_id": self.assistant_id,
                    "created_at": time.strftime("%Y-%m-%d %H:%M:%S")
                }
                with open(config_file, "w") as f:
                    json.dump(config_data, f, indent=4)
                print(f"DEBUG: Created new thread config: {config_file}")
        except Exception as e:
            print("DEBUG: Thread missing, creating new one...")
            self.thread = self.client.beta.threads.create()
            thread_id = self.thread.id
            
            # Save new thread config
            config_file = f"config_{self.assistant_id}.json"
            config_data = {
                "thread_id": thread_id,
                "assistant_id": self.assistant_id,
                "created_at": time.strftime("%Y-%m-%d %H:%M:%S")
            }
            with open(config_file, "w") as f:
                json.dump(config_data, f, indent=4)
            print(f"DEBUG: Created new thread config: {config_file}")

    def get_enhanced_instructions(self):
        """Combine base instructions with current context"""
        context = self.context_manager.build_context()
        return f"{self.base_instructions}\n\nCurrent Context:\n{context}"


    def stt(self, audio, language='en'):
        try:
            import wave
            from io import BytesIO

            wav_data = BytesIO(audio.get_wav_data())
            wav_data.name = self.STT_OUT

            transcript = self.client.audio.transcriptions.create(
                model="whisper-1", 
                file=wav_data,
                language=language,
                prompt="this is the conversation between me and a robot"
            )

            # file = "./stt_output.wav"
            # with wave.open(file, "wb") as wf:
            #     wf.write(audio.get_wav_data())

            # with open(file, 'rb') as f:
            #     transcript = client.audio.transcriptions.create(
            #         model="whisper-1", 
            #         file=f
            #     )
            return transcript.text
        except Exception as e:
            print(f"stt err:{e}")
            return False

    def speech_recognition_stt(self, recognizer, audio):
        import speech_recognition as sr

        # # recognize speech using Sphinx
        # try:
        #     print("Sphinx thinks you said: " + r.recognize_sphinx(audio, language="en-US"))
        # except sr.UnknownValueError:
        #     print("Sphinx could not understand audio")
        # except sr.RequestError as e:
        #     print("Sphinx error; {0}".format(e))

        # recognize speech using whisper
        # try:
        #     print("Whisper thinks you said: " + r.recognize_whisper(audio, language="english"))
        # except sr.UnknownValueError:
        #     print("Whisper could not understand audio")
        # except sr.RequestError as e:
        #     print(f"Could not request results from Whisper; {e}")

        # recognize speech using Whisper API
        try:
            return recognizer.recognize_whisper_api(audio, api_key=self.api_key)
        except sr.RequestError as e:
            print(f"Could not request results from Whisper API; {e}")
            return False

    def dialogue(self, msg: str):
        """Handle dialogue with context"""  
        try:
            chat_print("user", msg)
            
            # Create the message first
            print("DEBUG: Creating message...")
            message = self.client.beta.threads.messages.create(
                thread_id=self.thread.id,
                role="user",
                content=msg  # Send original message without context
            )

            print("DEBUG: Creating run...")
            # Create the run
            run = self.client.beta.threads.runs.create(
                thread_id=self.thread.id,
                assistant_id=self.assistant_id,
                instructions=self.get_enhanced_instructions()
            )

            # Poll for completion
            print("DEBUG: Starting polling...")
            while run.status in ["queued", "in_progress"]:
                time.sleep(0.5)  # Wait 500ms between checks
                run = self.client.beta.threads.runs.retrieve(
                    thread_id=self.thread.id,
                    run_id=run.id
                )

            if run.status == "completed":
                    # Get the latest message
                    print("DEBUG: Run completed, getting messages...")
                    messages = self.client.beta.threads.messages.list(
                        thread_id=self.thread.id
                    )
                    
                    if messages.data:
                        message_content = messages.data[0].content[0].text.value
                        print(f"DEBUG: Message content type: {type(message_content)}")
                        return str(message_content)  # Ensure we return a string
                    else:
                        print("DEBUG: No messages found")
                        return "No response received"
            else:
                print(f"DEBUG: Run failed with status: {run.status}")
                return "Error in processing response"
        except Exception as e:
            print(f"DEBUG: Error in chat process: {e}")
            return f"Error: {str(e)}"

    def dialogue_with_img(self, msg: str, img_path: str):
        """Handle dialogue with image and context"""
        try:
            print("DEBUG: Starting dialogue_with_img")
            
            # Check if thread exists, create if not
            if not hasattr(self, 'thread') or not self.thread:
                print("DEBUG: Creating new thread...")
                self.thread = self.client.beta.threads.create()
                print(f"DEBUG: New thread created with ID: {self.thread.id}")
            
            chat_print("user", msg)
            
            # Upload image file
            print("DEBUG: Uploading image file...")
            img_file = self.client.files.create(
                file=open(img_path, "rb"),
                purpose="vision"
            )
            print(f"DEBUG: Image uploaded with ID: {img_file.id}")

            # Create message with image
            print("DEBUG: Creating message with image...")
            message = self.client.beta.threads.messages.create(
                thread_id=self.thread.id,
                role="user",
                content=[
                    {
                        "type": "text",
                        "text": msg
                    },
                    {
                        "type": "image_file",
                        "image_file": {"file_id": img_file.id}
                    }
                ],
            )
            print(f"DEBUG: Message created with ID: {message.id}")

            # Create the run
            print("DEBUG: Creating run...")
            run = self.client.beta.threads.runs.create(
                thread_id=self.thread.id,
                assistant_id=self.assistant_id,
                instructions=self.get_enhanced_instructions()
            )
            print(f"DEBUG: Run created with ID: {run.id}")

            # Poll for completion
            print("DEBUG: Starting polling...")
            while run.status in ["queued", "in_progress"]:
                time.sleep(0.5)  # Wait 500ms between checks
                run = self.client.beta.threads.runs.retrieve(
                    thread_id=self.thread.id,
                    run_id=run.id
                )
                print(f"DEBUG: Run status: {run.status}")

            if run.status == "completed":
                # Get the latest message
                print("DEBUG: Run completed, getting messages...")
                messages = self.client.beta.threads.messages.list(
                    thread_id=self.thread.id
                )
                
                if messages.data:
                    message_content = messages.data[0].content[0].text.value
                    print(f"DEBUG: Got response content type: {type(message_content)}")
                    return str(message_content)  # Ensure we return a string
                else:
                    print("DEBUG: No messages found")
                    return "No response received"
            else:
                print(f"DEBUG: Run failed with status: {run.status}")
                return f"Error: Run failed with status {run.status}"
                
        except Exception as e:
            print(f"DEBUG: Error in dialogue_with_img: {e}")
            return f"Error: {str(e)}"


    def text_to_speech(self, text, output_file, voice='alloy', response_format="mp3", speed=1):
        '''
        voice: alloy, echo, fable, onyx, nova, and shimmer
        '''
        try:
            # check dir
            dir = os.path.dirname(output_file)
            if not os.path.exists(dir):
                os.mkdir(dir)
            elif not os.path.isdir(dir):
                raise FileExistsError(f"\'{dir}\' is not a directory")

            # tts
            with self.client.audio.speech.with_streaming_response.create(
                model="tts-1",
                voice=voice,
                input=text,
                response_format=response_format,
                speed=speed,
            ) as response:
                response.stream_to_file(output_file)

            return True
        except Exception as e:
            print(f'tts err: {e}')
            return False

class AssistantContextManager:
    """Manages context building for assistants"""
    def __init__(self, emotion_core: EmotionCore):
        self.memory_manager = emotion_core.get_memory_manager()
        grey_print(f"InitializingAssistantContextManager with memory_manager: {self.memory_manager} and emotion_core: {emotion_core}")
        
        self.emotion_core = emotion_core
        

    def build_context(self):
        """Build complete context for the assistant"""
        context_parts = []
        
        # Get memories from each enabled source
        for memory_type in self.memory_manager.memory_configs:
            memories = self.memory_manager.load_memory(memory_type)
            if memories:
                context_parts.append(f"[{memory_type.upper()} MEMORIES]: {memories}")
        
        # Add current mood if available
        try:
            current_mood = str(self.emotion_core.process_mood())
            if current_mood:
                context_parts.append(f"[CURRENT MOOD]: {current_mood}")
        except Exception as e:
            grey_print(f"Failed to get current mood: {e}")

        # Combine all context parts
        full_context = " ".join(context_parts)
        grey_print(f"Built context with {len(context_parts)} parts")
        return full_context

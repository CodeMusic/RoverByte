from fastapi import FastAPI, HTTPException, Request, Body, Response, UploadFile, File
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from fastapi.openapi.utils import get_openapi
from pydantic import BaseModel, Field
from typing import List, Optional, Dict, Union
from utils import debug_print
import uvicorn
import multiprocessing
import time
import json
import random
import asyncio
import logging
import signal
import sys
import os
import psutil
import threading
from concurrent.futures import ThreadPoolExecutor
import tempfile
import base64
import openai

# Setup logging with controlled verbosity
logging.basicConfig(
    level=logging.WARNING,  # Change to WARNING to reduce noise
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)

# Silence noisy loggers
logging.getLogger("openai").setLevel(logging.WARNING)
logging.getLogger("httpx").setLevel(logging.WARNING)
logging.getLogger("httpcore").setLevel(logging.WARNING)
logging.getLogger("asyncio").setLevel(logging.WARNING)
logging.getLogger("fastapi").setLevel(logging.WARNING)
logging.getLogger("uvicorn.access").setLevel(logging.WARNING)

# Keep RoverAPI logger at debug level
logger = logging.getLogger('RoverAPI')
logger.setLevel(logging.DEBUG)

# Data Models
class ChatMessage(BaseModel):
    role: str
    content: str

class ChatCompletionRequest(BaseModel):
    model: str
    messages: List[ChatMessage]
    temperature: Optional[float] = 0.7
    max_tokens: Optional[int] = None

class TTSRequest(BaseModel):
    text: str
    language: Optional[str] = "en"

class STTRequest(BaseModel):
    audio: str  # Base64 encoded audio data
    language: Optional[str] = "en"

# Helper Class for Interactions
class RoverInteraction:
    def __init__(self, openai_helper, dog_controller, ai_interaction):
        self.openai_helper = openai_helper
        self.dog_controller = dog_controller
        self.ai_interaction = ai_interaction
        self.thread_pool = ThreadPoolExecutor(max_workers=3)
        self.active_threads = []
        self.thread_lock = threading.Lock()
        self.speech_lock = threading.Lock()  # Add speech lock
        debug_print("RoverInteraction initialized")
        
        # Verify dog controller
        if self.dog_controller:
            debug_print("Dog controller available")
        else:
            debug_print("WARNING: Dog controller not available")

    def _play_speech(self, tts_file: str):
        """Play speech in thread with lock"""
        try:
            with self.speech_lock:  # Only one speech at a time
                debug_print(f"Playing speech file: {tts_file}")
                if hasattr(self.dog_controller, 'dog'):
                    self.dog_controller.dog.speak_block(tts_file)
                else:
                    debug_print("Dog controller not available")
        except Exception as e:
            debug_print(f"Speech playback error: {e}")

    async def execute_action(self, action: str) -> Dict:
        """Common method for executing actions"""
        try:
            if not self.dog_controller:
                return {"error": "Dog controller not available"}
                
            debug_print(f"Executing action: {action}")
            
            # Use thread instead of process
            future = self.thread_pool.submit(self._run_action, action)
            
            return {
                "message": f"Action initiated successfully",
                "action": action
            }
        except Exception as e:
            debug_print(f"Action execution error: {e}")
            return {"error": str(e)}

    def _run_action(self, action: str):
        """Execute action in thread"""
        try:
            debug_print(f"Running action: {action}")
            if hasattr(self.dog_controller, 'action_flow'):
                self.dog_controller.action_flow.run(action)
            else:
                debug_print("Action flow not available")
        except Exception as e:
            debug_print(f"Action execution error: {e}")

    async def execute_speech(self, text: str) -> Dict:
        """Common method for text-to-speech, with action support"""
        try:
            if not self.dog_controller:
                return {"error": "Dog controller not available"}
                
            debug_print(f"Processing speech with possible actions: {text}")
            
            # Parse for any actions in the text
            response_list = self.ai_interaction.parse_response(text)
            debug_print(f"Parsed response: {response_list}")
            
            results = []
            for item in response_list:
                if item.get('key') == 'action':
                    result = await self.execute_action(item.get('value', ''))
                    results.append(result)
                elif item.get('key') == 'content':
                    # Generate and play speech directly
                    tts_file = self.ai_interaction.generate_speech(item.get('value', ''))
                    if tts_file:
                        future = self.thread_pool.submit(self._play_speech, tts_file)
                        results.append({
                            "message": "Speech initiated successfully",
                            "text": item.get('value', ''),
                            "file": tts_file
                        })
            
            return {
                "message": "Speech processed successfully",
                "results": results
            }
                
        except Exception as e:
            debug_print(f"Speech processing error: {e}")
            return {"error": str(e)}

    async def process_interaction(self, text: str) -> Dict:
        """Process a full interaction with RoverByte"""
        try:
            if not self.dog_controller:
                return {"error": "Dog controller not available"}
            
            debug_print(f"Processing interaction: {text}")
            self.dog_controller.set_action_state('think')
            
            # Get AI response
            response = self.openai_helper.dialogue(text)
            debug_print(f"AI response: {response}")
            
            # Parse response
            response_list = self.ai_interaction.parse_response(response)
            debug_print(f"Parsed response: {response_list}")
            
            results = []
            # Process each part using common methods
            for item in response_list:
                if item.get('key') == 'action':
                    result = await self.execute_action(item.get('value', ''))
                    results.append(result)
                elif item.get('key') == 'content':
                    # Generate and play speech directly
                    tts_file = self.ai_interaction.generate_speech(item.get('value', ''))
                    if tts_file:
                        future = self.thread_pool.submit(self._play_speech, tts_file)
                        results.append({
                            "message": "Speech initiated successfully",
                            "text": item.get('value', ''),
                            "file": tts_file
                        })
            
            return {
                "message": "Interaction processed successfully",
                "input": text,
                "response": response,
                "results": results
            }
            
        except Exception as e:
            debug_print(f"Interaction processing error: {e}")
            return {"error": str(e)}

    def cleanup(self):
        """Cleanup thread pool"""
        self.thread_pool.shutdown(wait=False)


# Main Server Class
class OpenAIHelperServer:
    def __init__(self, openai_helper, dog_controller=None, ai_interaction=None):
        self.openai_helper = openai_helper
        self.dog_controller = dog_controller
        self.ai_interaction = ai_interaction
        self._start_time = time.time()
        
        # Initialize FastAPI
        self.app = FastAPI(
            title="RoverByte API",
            description="""
            RoverByte's API interface for direct interaction and control.
            
            Available Actions:
            - Sit
            - Stand
            - Wave
            - Dance
            - Stretch
            - Shake
            
            Features:
            - Natural language interaction
            - Direct action control
            - Text-to-speech output
            - Full interaction proxy
            """,
            version="1.0.0"
        )
        
        # Initialize interaction handler
        self.rover_interaction = RoverInteraction(openai_helper, dog_controller, ai_interaction)
        
        # Setup
        self.setup_middleware()
        self.setup_routes()
        self._setup_signal_handlers()

    def _setup_signal_handlers(self):
        signal.signal(signal.SIGINT, self._handle_shutdown)
        signal.signal(signal.SIGTERM, self._handle_shutdown)

    def _handle_shutdown(self, signum, frame):
        debug_print("Initiating graceful shutdown...")
        self.rover_interaction.cleanup()
        sys.exit(0)

    def setup_middleware(self):
        self.app.add_middleware(
            CORSMiddleware,
            allow_origins=["*"],
            allow_credentials=True,
            allow_methods=["*"],
            allow_headers=["*"],
        )

        @self.app.middleware("http")
        async def add_rate_limit(request: Request, call_next):
            # Skip rate limiting for OpenAPI docs and health checks
            if request.url.path in ["/openapi.json", "/docs", "/redoc", "/health"]:
                return await call_next(request)
                
            # Only rate limit the action endpoints
            if request.url.path in ["/rover/action", "/rover/speak", "/rover/proxy"]:
                if hasattr(self, '_last_request_time'):
                    time_since_last = time.time() - self._last_request_time
                    if time_since_last < 0.5:  # Minimum 0.5 seconds between requests
                        return JSONResponse(
                            status_code=429,
                            content={"error": "Too many requests, please wait"}
                        )
                self._last_request_time = time.time()
            
            return await call_next(request)

    def setup_routes(self):
        @self.app.post("/v1/chat/completions",
                      summary="OpenAI-compatible chat endpoint",
                      description="Get RoverByte's response without performing actions")
        async def chat_completions(request: ChatCompletionRequest):
            try:
                debug_print("Received chat completion request")
                user_message = request.messages[-1].content if request.messages else ""
                response = self.openai_helper.dialogue(user_message)
                
                return {
                    "id": str(random.randint(1000, 9999)),
                    "object": "chat.completion",
                    "created": time.time(),
                    "model": request.model,
                    "choices": [{"message": {"role": "assistant", "content": response}}],
                }
            except Exception as e:
                debug_print(f"Error in chat_completions: {e}")
                return {"error": str(e)}

        @self.app.post("/rover/action",
                      summary="Execute a specific action",
                      description="""
                      Make RoverByte perform an action.
                      
                      Available actions:
                      - sit
                      - stand
                      - wave
                      - dance
                      - stretch
                      - shake
                      - wag_tail
                      """)
        async def direct_action(action: str = Body(...)):
            return await self.rover_interaction.execute_action(action)

        @self.app.post("/rover/speak",
                      summary="Text-to-speech",
                      description="Make RoverByte speak the given text")
        async def speak_text(text: str = Body(...)):
            return await self.rover_interaction.execute_speech(text)

        @self.app.post("/rover/proxy",
                      summary="Full interaction proxy",
                      description="""
                      Process text as if spoken to RoverByte in person.
                      
                      This endpoint:
                      1. Processes the text through AI
                      2. Extracts actions and speech
                      3. Executes actions and generates speech
                      4. Returns the full interaction results
                      """)
        async def proxy_interaction(text: str = Body(...)):
            return await self.rover_interaction.process_interaction(text)

        @self.app.get("/health",
                      summary="Basic health check",
                      description="Check if the API server is running")
        async def health_check():
            return {
                "status": "healthy",
                "uptime": time.time() - self._start_time,
                "timestamp": time.time()
            }

        @self.app.get("/status",
                      summary="Detailed status",
                      description="""
                      Get detailed server status including:
                      - Uptime
                      - Memory usage
                      - Thread pool status
                      - Component availability
                      """)
        async def status():
            process = psutil.Process(os.getpid())
            return {
                "status": "healthy",
                "uptime": time.time() - self._start_time,
                "memory_usage": {
                    "rss": process.memory_info().rss / 1024 / 1024,  # MB
                    "vms": process.memory_info().vms / 1024 / 1024   # MB
                },
                "thread_pool_active": len(self.rover_interaction.thread_pool._threads),
                "dog_controller": "available" if self.dog_controller else "unavailable",
                "ai": "available" if self.ai_interaction else "unavailable"
            }

        @self.app.get("/version",
                      summary="API version information",
                      description="""
                      Get API version details including:
                      - Version number
                      - Available endpoints
                      - Server uptime
                      """)
        async def version():
            return {
                "version": "1.0.0",
                "endpoints": [
                    "/v1/chat/completions",
                    "/rover/action",
                    "/rover/speak",
                    "/rover/proxy",
                    "/health",
                    "/status",
                    "/version"
                ],
                "uptime": time.time() - self._start_time
            }

        @self.app.post("/rover/tts")
        async def text_to_speech(request: TTSRequest):
            try:
                # Get speech from OpenAI
                response = self.openai_helper.client.audio.speech.create(
                    model="tts-1",
                    voice="alloy",
                    input=request.text
                )
                
                # Convert binary audio to base64 string
                audio_base64 = base64.b64encode(response.content).decode('utf-8')
                
                # Return in MakeBlock's format
                return {
                    "code": 200,  # Success code
                    "data": {
                        "audio": audio_base64
                    },
                    "message": ""
                }
            except Exception as e:
                return {
                    "code": 50000,  # Error code
                    "data": {},
                    "message": str(e)
                }

        @self.app.post("/rover/stt")
        async def speech_to_text(request: Request):
            try:
                # Get raw audio data
                audio_data = await request.body()
                logger.info(f"Received audio data length: {len(audio_data)}")
                
                # Save temporarily (Whisper needs a file)
                with open("temp_audio.mp3", "wb") as f:
                    f.write(audio_data)
                
                """
                # Use Whisper for transcription
                with open("temp_audio.mp3", "rb") as audio_file:
                    transcript = self.openai_helper.client.audio.transcriptions.create(
                        model="whisper-1",
                        file=audio_file,
                        response_format="text"
                    )
                """
                
                transcript = "Test response"
                
                # Return in MakeBlock's format
                return {
                    "code": 20000,
                    "data": {
                        "text": transcript
                    },
                    "message": "success"
                }
            except Exception as e:
                logger.error(f"STT Error: {str(e)}")
                return {
                    "code": 50000,
                    "data": {},
                    "message": str(e)
                }

    def start(self):
        """Start the API server"""
        try:
            import uvicorn
            logger.info("Starting API server...")
            uvicorn.run(
                self.app,
                host="0.0.0.0",
                port=2345,
                log_level="info"
            )
        except Exception as e:
            logger.error(f"Error starting server: {e}")
            raise

    def stop(self):
        """Stop the API server"""
        logger.info("Stopping API server...")
        self.rover_interaction.cleanup()
        
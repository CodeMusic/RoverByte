import event
import time
import urequests
import ujson
from cyberpi import *

# initialize variables
OPENAI_API_KEY = ""

speech.set_recognition_address(url = "{NAVIGATEURL}")
speech.set_access_token(token = "{ACCESSTOKEN}")
driver.cloud_translate.TRANS_URL = "{TRANSURL}"
driver.cloud_translate.set_token("{ACCESSTOKEN}")
driver.cloud_translate.TTS_URL = "{TTSURL}"
driver.cloud_translate.set_token("{ACCESSTOKEN}")


base_url = "http://10.0.0.69:2345"
wifi_ssid = "RevivalNetwork "
wifi_password = "xunjmq84"

def connect_wifi():
    console.print("Connecting to WiFi...")
    
    if not wifi.is_connect():
        wifi.connect(wifi_ssid, wifi_password)
        while not wifi.is_connect():
            led.show_all(0, 0, 255)  # Blue while connecting
            time.sleep(0.5)
        led.show_all(0, 255, 0)  # Green when connected
    console.print("Connected!")
    time.sleep(0.1)

def check_server_health():
    try:
        response = urequests.get(base_url + "/health")
        if response.status_code == 200:
            data = response.json()
            if data.get('status') == 'healthy':
                led.show_all(0, 255, 0)  # Solid green for healthy
                return True
        return False
    except Exception as e:
        return False

def blink_error():
    for _ in range(3):  # Blink 3 times
        led.show_all(255, 0, 0)  # Red
        time.sleep(0.5)
        led.show_all(0, 0, 0)  # Off
        time.sleep(0.5)


def voice_to_text():
    console.clear()
    console.print("Press A to start recording")
    console.print("Press A again to stop")
    
    # Point to our STT endpoint
    speech.set_recognition_address(url = "http://10.0.0.69:2345/rover/stt")
    speech.set_access_token(token = OPENAI_API_KEY)  # Using our OpenAI key
    
    led.show_all(0, 255, 0)  # Green for ready
    
    # Wait for A press to start
    while not controller.is_press("a"):
        time.sleep(0.1)
    time.sleep(0.2)  # Debounce
    
    # Start recording
    console.print("Recording... Press A to stop")
    led.show_all(255, 0, 0)  # Red while recording
    audio.record()
    
    # Wait for A press to stop
    while not controller.is_press("a"):
        time.sleep(0.1)
    time.sleep(0.2)  # Debounce
    
    # Stop recording and process
    audio.stop_record()
    led.show_all(255, 255, 0)  # Yellow while processing
    console.print("Processing speech...")
    
    try:
        # Send recording for recognition while playing it
        cloud.listen('english', 1)
        console.print("Sent to our server...")
        time.sleep(1)
        
        result = cloud.listen_result()
        console.print("Raw result: " + str(result))
        
        if result:
            recorded_text = str(result)
            console.print("Got text: " + recorded_text)
        else:
            console.print("No speech detected")
            recorded_text = ""
            
    except Exception as e:
        console.print("Error type: " + str(type(e)))
        console.print("Error msg: " + str(e))
        recorded_text = ""
    
    led.show_all(0, 255, 0)  # Green when done    
    time.sleep(1)
    return recorded_text


def retry_connection():
    console.clear()
    console.print("Connection Failed!")
    console.print("\nPress A to retry connection")
    led.show_all(255, 0, 0)  # Red LED
    
    while True:
        if controller.is_press("a"):
            audio.play("magic")
            console.clear()
            console.print("Retrying connection...")
            if not wifi.is_connect():
                connect_wifi()

            if check_server_health():
                led.show_all(0, 255, 0)  # Green LED
                speak_cyberpi("RoverRemote is ready!")
                return True
            else:
                blink_error()
                console.clear()
                console.print("Unable to reach Rover.")
                console.print("\nPress A to retry connection")
                time.sleep(1)
        time.sleep(0.1)

def speak_cyberpi(text):
    console.print(">> " + str(text))
    cloud.tts('en', text)

def api_speak(text):
    console.print("Speaking: " + str(text))
    try:
        response = urequests.post(
            base_url + "/rover/speak",
            headers={'Content-Type': 'application/json'},
            json=text
        )
        returnValue = response.json() if response.status_code == 200 else "Error: " + str(response.status_code)
        console.print("API: " + str(returnValue))
        return returnValue
    except Exception as e:
        console.print("Error: " + str(e))

def api_send_command(command):
    try:
        console.print("Command: " + str(command))
        console.print("Sending to: " + base_url + "/rover/action")
        response = urequests.post(
            base_url + "/rover/action",
            headers={'Content-Type': 'application/json'},
            json=command
        )
        returnValue = response.json() if response.status_code == 200 else "Error: " + str(response.status_code)
        console.print("API: " + str(returnValue))
        return returnValue
    except Exception as e:
        return "Error: " + str(e)

def api_send_proxy_request(request):
    try:
        response = urequests.post(
            base_url + "/rover/proxy",
            headers={'Content-Type': 'application/json'},
            json=request
        )
        returnValue = response.json() if response.status_code == 200 else "Error: " + str(response.status_code)
        console.print("API: " + str(returnValue))
        return returnValue
    except Exception as e:
        return "Error: " + str(e)

def api_chat_completion(input_text):
    payload = {
        "model": "codemusai",
        "messages": [{"role": "user", "content": input_text}],
        "temperature": 0.7,
        "max_tokens": 150
    }
    try:
        response = urequests.post(
            base_url + "/v1/chat/completions",
            headers={'Content-Type': 'application/json'},
            json=payload
        )
        if response.status_code == 200:
            return response.json()['choices'][0]['message']['content']
        else:
            return "Error: " + str(response.status_code)
    except Exception as e:
        return "Error: " + str(e)

def display_main_menu(selected_index):
    console.clear()
    console.print("RoverRemote:\n")
    
    menu_items = [
        "Actions",  # Menu of predefined actions
        "Command",  # Voice command
        "Speak",
        "Proxy",
        "Chat",
        "Quick Commands"  # New menu item
    ]
    
    for i, item in enumerate(menu_items):
        if i == selected_index:
            console.print(">" + item + "\n")  # Show cursor for selected item
        else:
            console.print(" " + item + "\n")

def display_actions_menu(selected_index):
    console.clear()
    console.print("Rover Actions:\n")
    
    actions = [
        "BACK <<",  # First option to return to main menu
        "forward",
        "backward",
        "lie",
        "stand",
        "sit",
        "bark",
        "bark harder",
        "pant",
        "howling",
        "wag_tail",
        "stretch",
        "push up",
        "scratch",
        "handshake",
        "high five",
        "lick hand",
        "shake head",
        "relax neck",
        "nod",
        "think",
        "recall",
        "head down",
        "fluster",
        "surprise",
        "dab",
        "floss",
        "woah",
        "gangnam style",
        "bottle flip",
        "twerk",
        "pray",
        "butt up"
    ]
    
    # Calculate which portion of the menu to show (for scrolling)
    start_idx = max(0, selected_index - 3)
    end_idx = min(len(actions), start_idx + 8)
    
    if selected_index > 3:
        console.print(" ↑ More actions above")
    
    for i in range(start_idx, end_idx):
        if i == selected_index:
            console.print(">" + actions[i])
        else:
            console.print(" " + actions[i])
    
    if end_idx < len(actions):
        console.print(" ↓ More actions below")
    
    return actions

def configure_settings():
    useBuiltIn = True
    if useBuiltIn:
        console.print("Using built-in speech components")
        speech.set_recognition_address(url = "wss://mindplus.makeblock.com/mbapi/audio2text")
        speech.set_access_token(token = "{ACCESSTOKEN}")
        driver.cloud_translate.TRANS_URL = "wss://mindplus.makeblock.com/mbapi/translate"
        driver.cloud_translate.set_token("{ACCESSTOKEN}")
        driver.cloud_translate.TTS_URL = "wss://mindplus.makeblock.com/mbapi/text2audio"
        driver.cloud_translate.set_token("{ACCESSTOKEN}")
    else:
        console.print("Using custom speech components")
        speech.set_recognition_address(url = "http://10.0.0.69:2345/rover/stt")
        speech.set_access_token(token = "{ACCESSTOKEN}")
        driver.cloud_translate.TRANS_URL = "{TRANSURL}"
        driver.cloud_translate.set_token("{ACCESSTOKEN}")
        driver.cloud_translate.TTS_URL = "http://10.0.0.69:2345/rover/tts"
        driver.cloud_translate.set_token("{ACCESSTOKEN}")

def handle_quick_commands():
    console.clear()
    console.print("Quick Commands Mode")
    console.print("Up: Stand")
    console.print("Down: Lie")
    console.print("Left: Push Up")
    console.print("Right: Sit")
    console.print("Click: Bark")
    console.print("A: Howl")
    console.print("B: Exit")
    
    while True:
        try:
            if controller.is_press("up"):
                response = api_send_command("stand")
                console.print("Sent: stand")
                time.sleep(0.5)
                
            elif controller.is_press("down"):
                response = api_send_command("lie")
                console.print("Sent: lie")
                time.sleep(0.5)
                
            elif controller.is_press("left"):
                response = api_send_command("push up")
                console.print("Sent: push up")
                time.sleep(0.5)
                
            elif controller.is_press("right"):
                response = api_send_command("sit")
                console.print("Sent: sit")
                time.sleep(0.5)
                
            elif controller.is_press("middle"):  # Joystick click
                response = api_send_command("bark")
                console.print("Sent: bark")
                time.sleep(0.5)
            
            elif controller.is_press("a"):
                response = api_send_proxy_request("Hey Rover, tell me a random fact, or joke, or story, but make it relate to what we have discusssed, but with some randomness.");
                console.print("Sent: Random AI")
                time.sleep(0.5)
                
            elif controller.is_press("b"):
                break
                
        except Exception as e:
            console.print("Error: " + str(e))
            if not retry_connection():
                break
        
        time.sleep(0.1)  # Small delay to prevent button spam

@event.start
def on_start():
    configure_settings()
    speech.set_recognition_address(url = "http://msapi.passport3.makeblock.com/ms/bing_speech/interactive")
    speech.set_access_token(token = "KEY")

    console.clear()
    #..cloud.tts_set_url(("http://10.0.0.69:2345/rover/tts")) #recognition_set_url
    audio.set_vol(91)
    time.sleep(0.1)
    connect_wifi()
    console.clear()
    speak_cyberpi("Starting RoverRemote...")

    # Keep trying until connection works
    while not check_server_health():
        if not retry_connection():
            continue

    # Only show menu options after successful connection
    while True:
        selected_index = 0
        menu_size = 6
        
        # Menu navigation loop
        while True:
            display_main_menu(selected_index)
            
            # Wait for input
            while not (controller.is_press("up") or 
                      controller.is_press("down") or 
                      controller.is_press("a")):
                time.sleep(0.1)
            
            if controller.is_press("up"):
                #audio.play("drop")
                selected_index = (selected_index - 1) % menu_size
                time.sleep(0.1)
            elif controller.is_press("down"):
                #audio.play("drop")
                selected_index = (selected_index + 1) % menu_size
                time.sleep(0.1)
            elif controller.is_press("a"):
                audio.play("magic")
                break
        
        # Handle menu selection
        try:
            if selected_index == 0:  # Actions menu
                action_index = 0
                actions = display_actions_menu(action_index)
                action_menu_size = len(actions)
                
                # Action selection loop
                while True:
                    display_actions_menu(action_index)
                    
                    while not (controller.is_press("up") or 
                              controller.is_press("down") or 
                              controller.is_press("a")):
                        time.sleep(0.1)
                    
                    if controller.is_press("up"):
                        #audio.play("drop")
                        action_index = (action_index - 1) % action_menu_size
                        time.sleep(0.1)
                    elif controller.is_press("down"):
                        #audio.play("drop")
                        action_index = (action_index + 1) % action_menu_size
                        time.sleep(0.1)
                    elif controller.is_press("a"):
                        audio.play("magic")
                        if action_index == 0:  # Back option
                            break
                        else:
                            selected_action = actions[action_index]
                            response = api_send_command(selected_action)
                            console.clear()
                            console.print(selected_action + " sent")
                            speak_cyberpi(response)
                            time.sleep(0.1)
                            break
            
            elif selected_index == 1:  # Voice Command
                voice_input = voice_to_text()
                if voice_input:
                    response = api_send_command(voice_input)
                    console.clear()
                    console.print("Command sent: " + voice_input)
                time.sleep(1)
            elif selected_index == 2:  # Speak
                voice_input = voice_to_text()
                if voice_input:
                    response = api_speak(voice_input)
            elif selected_index == 3:  # Proxy
                voice_input = voice_to_text()
                if voice_input:
                    response = api_send_proxy_request(voice_input)
            elif selected_index == 4:  # Chat
                voice_input = voice_to_text()
                if voice_input:
                    response = api_chat_completion(voice_input)        
            elif selected_index == 5:  # Quick Commands
                handle_quick_commands()
                continue  # Skip response handling
                
        except Exception as e:
            # If any operation fails, it might be a connection issue
            console.print("Error: " + str(e))
            if not retry_connection():
                continue

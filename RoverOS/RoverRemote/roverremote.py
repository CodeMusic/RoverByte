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
        cloud.listen('english', t=1)
        time.sleep(1)
        
        result = cloud.listen_result()
        console.print("Raw result: " + str(result))
        
        if result:
            recorded_text = str(result)
            console.print("Got text: " + recorded_text)
        else:
            console.print("No speech detected")
            recorded_text = "Hi Chris, I didn't hear you."
            
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
        "Chat"
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
        "BACK <<\n",  # First option to return to main menu
        "forward\n",
        "backward\n",
        "lie\n",
        "stand\n",
        "sit\n",
        "bark\n",
        "bark harder\n",
        "pant\n",
        "howling\n",
        "wag_tail\n",
        "stretch\n",
        "push up\n",
        "scratch\n",
        "handshake\n",
        "high five\n",
        "lick hand\n",
        "shake head\n",
        "relax neck\n",
        "nod\n",
        "think\n",
        "recall\n",
        "head down\n",
        "fluster\n",
        "surprise\n",
        "dab\n",
        "floss\n",
        "woah\n",
        "gangnam style\n",
        "bottle flip\n",
        "twerk\n",
        "pray\n",
        "butt up\n"
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
        cyberpi.speech.set_recognition_address(url = "wss://mindplus.makeblock.com/mbapi/audio2text")
        cyberpi.speech.set_access_token(token = "{ACCESSTOKEN}")
        cyberpi.driver.cloud_translate.TRANS_URL = "wss://mindplus.makeblock.com/mbapi/translate"
        cyberpi.driver.cloud_translate.set_token("{ACCESSTOKEN}")
        cyberpi.driver.cloud_translate.TTS_URL = "wss://mindplus.makeblock.com/mbapi/text2audio"
        cyberpi.driver.cloud_translate.set_token("{ACCESSTOKEN}")
    else:
        cyberpi.speech.set_recognition_address(url = "{NAVIGATEURL}")
        cyberpi.speech.set_access_token(token = "{ACCESSTOKEN}")
        cyberpi.driver.cloud_translate.TRANS_URL = "{TRANSURL}"
        cyberpi.driver.cloud_translate.set_token("{ACCESSTOKEN}")
        cyberpi.driver.cloud_translate.TTS_URL = "http://10.0.0.69:2345/rover/tts"
        cyberpi.driver.cloud_translate.set_token("{ACCESSTOKEN}")

@event.start
def on_start():
    #configure_settings()
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
        menu_size = 5
        
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
        except Exception as e:
            # If any operation fails, it might be a connection issue
            console.print("Error: " + str(e))
            if not retry_connection():
                continue

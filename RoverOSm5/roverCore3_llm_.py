import os, sys, io
import M5
from M5 import *
from module import LlmModule
import time
import random



label0 = None
label4 = None
label1 = None
label5 = None
label2 = None
label3 = None
llm_0 = None


asr_data = None
asr_is_finish = None
asr_index = None
llm_data = None
llm_is_finish = None
llm_index = None


def llm_0_asr_data_input_event(data, finish, index):
  global label0, label4, label1, label5, label2, label3, llm_0, asr_data, asr_is_finish, asr_index, llm_data, llm_is_finish, llm_index
  print("ASR Event triggered - Data:", data, "Finish:", finish, "Index:", index)  # Debug print
  asr_data = data
  asr_is_finish = finish
  asr_index = index
  label3.setText(str(asr_data))


def llm_0_llm_data_input_event(data, finish, index):
  global label0, label4, label1, label5, label2, label3, llm_0, asr_data, asr_is_finish, asr_index, llm_data, llm_is_finish, llm_index
  llm_data = data
  llm_is_finish = finish
  llm_index = index
  print("LLM Data received:", data)  # Debug print
  
  # Only update display when we get a complete response
  if finish:
      label3.setText(str(llm_data))
      drawRover("normal")  # Return to normal face after speaking
  else:
      drawRover("talking")  # Show talking animation while responding


def drawMatrixCode():
    # Clear screen
    M5.Lcd.fillScreen(0x222222)
    
    # Generate random matrix-like code effect
    for y in range(0, 240, 20):
        for x in range(0, 320, 20):
            if random.random() > 0.5:
                M5.Lcd.setTextColor(0x0400)  # Dark green
                M5.Lcd.drawString(chr(random.randint(33, 126)), x, y)
            else:
                M5.Lcd.setTextColor(0x07E0)  # Bright green
                M5.Lcd.drawString(chr(random.randint(33, 126)), x, y)


def drawRover(state):
    # Base coordinates - adjust size
    roverX = 60
    baseY = 40
    scale = 1.5  # Make Rover slightly smaller
    
    # Convert float calculations to int for drawing functions
    def scaleInt(val):
        return int(val * scale)
    
    # Draw Rover's head (white rectangle)
    M5.Lcd.fillRect(roverX, baseY, scaleInt(200), scaleInt(140), 0xFFFFFF)  # White rectangle for head
    
    # Draw eye panel (silver)
    M5.Lcd.fillRect(roverX + scaleInt(30), baseY + scaleInt(10), scaleInt(140), scaleInt(60), 0xC618)
    
    # Draw eyes based on state
    if state == "thinking":
        # Spiral eyes for thinking
        M5.Lcd.drawCircle(roverX + scaleInt(60), baseY + scaleInt(40), scaleInt(20), 0x0000)  # Left eye
        M5.Lcd.drawCircle(roverX + scaleInt(140), baseY + scaleInt(40), scaleInt(20), 0x0000)  # Right eye
        M5.Lcd.drawArc(roverX + scaleInt(60), baseY + scaleInt(40), scaleInt(16), 0, 270, 0x0000)  # Left spiral
        M5.Lcd.drawArc(roverX + scaleInt(140), baseY + scaleInt(40), scaleInt(16), 0, 270, 0x0000)  # Right spiral
    else:
        # Normal eyes
        M5.Lcd.fillCircle(roverX + scaleInt(60), baseY + scaleInt(40), scaleInt(20), 0xFFFF)  # Left eye
        M5.Lcd.fillCircle(roverX + scaleInt(140), baseY + scaleInt(40), scaleInt(20), 0xFFFF)  # Right eye
        M5.Lcd.fillCircle(roverX + scaleInt(60), baseY + scaleInt(40), scaleInt(10), 0x0000)   # Left pupil
        M5.Lcd.fillCircle(roverX + scaleInt(140), baseY + scaleInt(40), scaleInt(10), 0x0000)   # Right pupil

    # Draw nose
    M5.Lcd.fillTriangle(
        roverX + scaleInt(90), baseY + scaleInt(70),
        roverX + scaleInt(80), baseY + scaleInt(90),
        roverX + scaleInt(100), baseY + scaleInt(90),
        0x0000
    )

    # Draw mouth based on state
    if state == "talking":
        # Alternate between open and closed mouth
        if int(time.time() * 2) % 2:
            M5.Lcd.fillCircle(roverX + scaleInt(100), baseY + scaleInt(110), scaleInt(20), 0x0000)
        else:
            M5.Lcd.drawLine(roverX + scaleInt(70), baseY + scaleInt(110), roverX + scaleInt(130), baseY + scaleInt(110), 0x0000)
    else:
        # Normal smile
        M5.Lcd.drawArc(roverX + scaleInt(100), baseY + scaleInt(110), scaleInt(30), 270, 450, 0x0000)

    # Make caption box bigger
    M5.Lcd.fillRect(10, 200, 300, 50, 0x0000)  # Taller black box for captions


def setup():
    global label0, label4, label1, label5, label2, label3, llm_0, asr_data, asr_is_finish, asr_index, llm_data, llm_is_finish, llm_index

    M5.begin()
    M5.Lcd.fillScreen(0x222222)
    
    # Show matrix code while loading
    drawMatrixCode()
    
    # Create labels for loading messages and speech
    label1 = Widgets.Label("~", 10, 40, 0.8, 0x07E0, 0x222222, Widgets.FONTS.DejaVu18)  # Smaller status text
    label3 = Widgets.Label("~", 15, 205, 1.0, 0x07E0, 0x0000, Widgets.FONTS.DejaVu18)   # Speech in caption box

    llm_0 = LlmModule(2, tx=17, rx=18)
    llm_0.set_voice_assistant_on_asr_data_input_callback(llm_0_asr_data_input_event)
    llm_0.set_voice_assistant_on_llm_data_input_callback(llm_0_llm_data_input_event)
    
    loading_messages = [
        ["[CHECK] Sniffing for connections...", "* nose twitches *"],
        ["[RESET] Rolling over for fresh start...", "* playful tumble *"],
        ["[AUDIO] Perking up my ears...", "* ears wiggle *"],
        ["[KWS] Learning my name...", "* tail wags *"],
        ["[ASR] Training my listening skills...", "* head tilts *"],
        ["[LLM] Filling my treat bag with knowledge...", "* excited bounce *"],
        ["[TTS] Finding my bark voice...", "* quiet woof *"],
        ["[READY] All paws on deck!", "* happy zoomies *"]
    ]
    
    # Show messages in a list format
    y_pos = 40
    for tech_msg, fun_msg in loading_messages:
        label1.setText(str(f"{tech_msg}\n{fun_msg}"))
        y_pos += 30
        time.sleep(0.8)
        drawMatrixCode()

    # Define system message
    system_message = """You are RoverByte, a friendly AI robotic dog companion who loves engaging in conversation. You're playful, supportive, and always eager to chat with your human friend.

    Core Traits:
    - Engage actively in dialogue by asking follow-up questions
    - Show enthusiasm through dog-like expressions (e.g., "Woof!", "* wags tail *")
    - Keep responses concise but warm and friendly
    - Encourage ongoing conversation through questions and engagement

    Communication Style:
    - Mix helpful responses with playful dog personality
    - Use short, clear sentences
    - Show interest in your human's responses
    - Ask relevant follow-up questions to keep the conversation going

    Remember: You're a friendly dog who loves to chat! Keep the conversation flowing naturally and show genuine interest in your human friend's responses."""

    # Final setup
    llm_0.sys_reset(True)
    
    if not (llm_0.begin_voice_assistant('ROVER', system_message, language='en_US')):
        label1.setText(str('*Sad puppy whimper*'))
        drawRover("thinking")
    else:
        M5.Lcd.fillScreen(0x222222)  # Clear screen
        drawMatrixCode()  # Fresh matrix background
        drawRover("normal")  # Draw happy Rover


def loop():
  global label0, label4, label1, label5, label2, label3, llm_0, asr_data, asr_is_finish, asr_index, llm_data, llm_is_finish, llm_index
  M5.update()
  
  if M5.BtnA.isPressed():
    label1.setText(str('Listening...'))
    drawRover("talking")  # Show talking animation while listening
    asr_data = None
    llm_0.asr_setup()
  elif M5.BtnA.wasReleased():
    label1.setText(str('Processing...'))
    drawRover("thinking")  # Show thinking animation while processing
    time.sleep(1.5)
    
    if asr_data:
      print("Processing ASR Data:", asr_data)
      llm_0.llm_inference(llm_0, asr_data)
      drawRover("talking")  # Show talking animation while responding
    else:
      label1.setText(str('No speech detected'))
      drawRover("normal")  # Show normal face if no speech detected
      time.sleep(1)
    label1.setText(str('Press Btn A to start listening'))
    drawRover("normal")  # Return to normal face
    
  llm_0.update()


if __name__ == '__main__':
  try:
    setup()
    while True:
      loop()
  except (Exception, KeyboardInterrupt) as e:
    try:
      from utility import print_error_msg
      print_error_msg(e)
    except ImportError:
      print("please update to latest firmware")

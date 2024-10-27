from ai_interaction import AIInteraction

"""
This file contains the voice commands for the roverbyte where the result is executed but not sent to the AI.
"""

VOICE_COMMANDS = {
    # Movement
    "forward": "forward",
    "backward": "backward",
    
    # Posture
    "lie down": "lie",
    "stand": "stand",
    "sit": "sit",
    
    # Sounds
    "bark": "bark",
    "bark harder": "bark_harder",
    "pant": "pant",
    "howl": "howling",
    
    # Expressions
    "wag tail": "wag_tail",
    "stretch": "stretch",
    "push up": "push_up",
    "scratch": "scratch",
    "shake hands": "hand_shake",
    "high five": "high_five",
    "lick hand": "lick_hand",
    "shake head": "shake_head",
    "relax neck": "relax_neck",
    "nod": "nod",
    "think": "think",
    "recall": "recall",
    "head down": "head_down",
    "fluster": "fluster",
    "surprise": "surprise",
    
    # New meme-inspired actions
    "dab": "dab",
    "floss": "floss",
    "whoa": "woah",
    "gangnam style": "gangnam_style",
    "bottle flip": "bottle_flip",
    "twerk": "twerk",
    "pray": "pray",
    "butt up": "butt_up",
}

def process_voice_command(text, ai, dog):
    command = text.lower()
    if command in VOICE_COMMANDS:
        action = VOICE_COMMANDS[command]
        dog.action_flow.run(action)
        return True
    else:
        # If not a direct command, pass to AI
        ai_response = ai.generate_response(text)
        # Check if AI response contains an action
        for cmd in VOICE_COMMANDS.values():
            if cmd in ai_response.lower():
                dog.action_flow.run(cmd)
                return True
        # If no action in AI response, just speak the response
        dog.speak(ai_response)
        return False
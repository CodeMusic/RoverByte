from pydub import AudioSegment
import os

def identify_audio_file(file_path):
    try:
        audio = AudioSegment.from_file(file_path)
        return audio.format_info
    except Exception as e:
        return f"Not a recognized audio file: {str(e)}"


# Example usage
file_path = '/Users/christopherhicks/Projects/RoverByte/RoverOS/Tests/temp_audio.mp3'
audio_type = identify_audio_file(file_path)
print(f"The audio file type is: {audio_type}")
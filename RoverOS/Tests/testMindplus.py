import websockets
import asyncio
import json

async def test_tts():
    uri = "wss://mindplus.makeblock.com/mbapi/text2audio"
    headers = {
        "Authorization": "Bearer q373ym0bo17"
    }
    
    async with websockets.connect(uri, extra_headers=headers) as websocket:
        message = {
            "text": "hello world",
            "language": "en"
        }
        await websocket.send(json.dumps(message))
        response = await websocket.recv()
        
        # Save the response to a file
        with open("test.mp3", "wb") as f:
            f.write(response)

asyncio.get_event_loop().run_until_complete(test_tts())
import requests
import time

def test_endpoints():
    base_url = "http://roverbyte.local:2345"
    
    # Test health
    health = requests.get(f"{base_url}/health")
    print(f"Health check: {health.json()}")
    
    # Test chat
    chat = requests.post(f"{base_url}/v1/chat/completions",
        json={
            "model": "gpt-3.5-turbo",
            "messages": [{"role": "user", "content": "Hello!"}]
        })
    print(f"Chat response: {chat.json()}")
    
    # More tests...

if __name__ == "__main__":
    test_endpoints()
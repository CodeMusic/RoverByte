import os, sys, io
import M5
from M5 import *
import time
import network
import requests
import json

# E-paper display constants
BLACK = 0  # Black pixels
WHITE = 1  # White pixels (background)
GREY = 2   # For partial refresh/grey

# Configuration
WIFI_SSID = "RevivalNetwork "
WIFI_PASSWORD = "xunjmq84"

# Google OAuth Configuration
CLIENT_ID = ""

class ProjectTasks:
    def __init__(self):
        self.lists = []
        self.tasks = {}
        self.current_list_index = 0
        self.current_task_index = 0
        self.view_mode = "lists"
        self.scroll_position = 0
        self.max_items_per_page = 8
        self.access_token = None
        self.device_code = None
        self.wlan = None

    def try_auth_with_scope(self, scope):
        try:
            auth_url = "https://oauth2.googleapis.com/device/code"
            post_data = {
                'client_id': CLIENT_ID,
                'scope': scope
            }
            
            headers = {
                'Content-Type': 'application/json',
                'Accept': 'application/json'
            }
            
            response = requests.post(
                auth_url,
                json=post_data,
                headers=headers,
                timeout=10
            )
            
            return response
            
        except Exception as e:
            return None

    def start_device_flow(self):
        try:
            Lcd.clear()
            Lcd.setTextColor(BLACK)
            Lcd.setTextSize(2)
            Lcd.drawString("Starting auth...", 20, 120)
            
            # Device OAuth flow with exact scope from console
            auth_url = "https://oauth2.googleapis.com/device/code"
            post_data = {
                'client_id': CLIENT_ID,
                'scope': 'https://www.googleapis.com/auth/tasks'
            }
            
            headers = {
                'Content-Type': 'application/json',
                'Accept': 'application/json'
            }
            
            Lcd.drawString("Sending request...", 20, 160)
            
            try:
                response = requests.post(
                    auth_url,
                    json=post_data,
                    headers=headers,
                    timeout=10
                )
                
                # Debug response
                Lcd.clear()
                Lcd.drawString(f"Status: {response.status_code}", 20, 120)
                
                if response.status_code == 200:
                    data = response.json()
                    self.device_code = data["device_code"]
                    user_code = data["user_code"]
                    verification_url = data["verification_url"]
                    
                    Lcd.clear()
                    Lcd.drawString("To authorize:", 20, 40)
                    Lcd.drawString("1. Go to:", 20, 80)
                    Lcd.drawString(verification_url, 20, 120)
                    Lcd.drawString("2. Enter code:", 20, 160)
                    Lcd.drawString(user_code, 20, 200)
                    Lcd.drawString("Press side button", 20, 280)
                    Lcd.drawString("when done", 20, 320)
                    return True
                else:
                    Lcd.clear()
                    Lcd.drawString("Auth Failed:", 20, 120)
                    Lcd.drawString(f"Status: {response.status_code}", 20, 160)
                    try:
                        error_data = response.json()
                        error_msg = error_data.get('error', 'Unknown error')
                        error_desc = error_data.get('error_description', '')
                        Lcd.drawString(f"Error: {error_msg}", 20, 200)
                        if error_desc:
                            for i, line in enumerate(range(0, len(error_desc), 30)):
                                Lcd.drawString(error_desc[line:line+30], 20, 240 + (i * 40))
                    except:
                        Lcd.drawString(response.text[:30], 20, 200)
                    time.sleep(3)
                    return False
                    
            except Exception as e:
                Lcd.clear()
                Lcd.drawString("Request Failed:", 20, 120)
                error_str = str(e)
                for i, line in enumerate(range(0, len(error_str), 30)):
                    Lcd.drawString(error_str[line:line+30], 20, 160 + (i * 40))
                time.sleep(3)
                return False
                
        except Exception as e:
            Lcd.clear()
            Lcd.drawString("Auth Error:", 20, 120)
            error_str = str(e)
            for i, line in enumerate(range(0, len(error_str), 30)):
                Lcd.drawString(error_str[line:line+30], 20, 160 + (i * 40))
            time.sleep(3)
            return False

    def connect_wifi(self):
        Lcd.clear()
        Lcd.setTextColor(BLACK)
        Lcd.setTextSize(2)
        Lcd.drawString("Connecting to WiFi...", 20, 120)
        
        try:
            if self.wlan is not None:
                self.wlan.active(False)
                time.sleep(1)
            
            self.wlan = network.WLAN(network.STA_IF)
            self.wlan.active(False)
            time.sleep(1)
            self.wlan.active(True)
            time.sleep(1)
            
            if not self.wlan.isconnected():
                self.wlan.connect(WIFI_SSID, WIFI_PASSWORD)
                attempts = 0
                while not self.wlan.isconnected() and attempts < 20:
                    time.sleep(1)
                    attempts += 1
                    Lcd.drawString("." * attempts, 20, 200)
            
            if self.wlan.isconnected():
                ip = self.wlan.ifconfig()[0]
                Lcd.clear()
                Lcd.drawString("Connected!", 20, 120)
                Lcd.drawString(f"IP: {ip}", 20, 160)
                time.sleep(2)
                return True
            
            self.wlan.active(False)
            Lcd.clear()
            Lcd.drawString("Failed to connect!", 20, 120)
            time.sleep(2)
            return False
            
        except Exception as e:
            Lcd.clear()
            Lcd.drawString("WiFi Error:", 20, 120)
            Lcd.drawString(str(e), 20, 160)
            time.sleep(2)
            return False

    def cleanup(self):
        if self.wlan:
            self.wlan.active(False)

    def draw_lists_screen(self):
        Lcd.clear()
        
        # Draw header
        Lcd.fillRect(0, 0, 540, 50, WHITE)  # White background
        Lcd.setTextColor(BLACK)
        Lcd.setTextSize(2)
        Lcd.drawString("Projects", 20, 20)
        
        # Draw list items
        y = 70
        for i in range(self.scroll_position, min(self.scroll_position + self.max_items_per_page, len(self.lists))):
            if i == self.current_list_index:
                Lcd.fillRect(10, y-5, 520, 40, BLACK)  # Selected item background
                Lcd.setTextColor(WHITE)  # White text for selected item
            else:
                Lcd.setTextColor(BLACK)  # Black text for unselected items
            
            title = self.lists[i].get("title", "")
            if len(title) > 35:
                title = title[:32] + "..."
            Lcd.drawString(title, 20, y)
            
            if self.lists[i]["id"] in self.tasks:
                count = str(len(self.tasks[self.lists[i]["id"]]))
                Lcd.drawString(f"({count})", 480, y)
            
            y += 45

def setup():
    M5.begin()
    Lcd.setTextColor(BLACK)  # Default to black text
    Lcd.setTextSize(1)

def main():
    setup()
    app = ProjectTasks()
    
    try:
        # Show startup screen
        Lcd.clear()
        Lcd.setTextSize(2)
        Lcd.drawString("Starting...", 20, 120)
        time.sleep(1)
        
        # Connect to WiFi with retries
        wifi_attempts = 0
        while wifi_attempts < 3:
            if app.connect_wifi():
                break
            wifi_attempts += 1
            time.sleep(2)
        
        if wifi_attempts == 3:
            Lcd.clear()
            Lcd.drawString("WiFi failed after 3 attempts", 20, 120)
            time.sleep(2)
            return

        # Start Google OAuth device flow
        if app.start_device_flow():
            # Wait for user to complete authorization
            while True:
                M5.update()
                if BtnP.wasPressed():  # Side button pressed
                    break
                time.sleep(0.1)
        else:
            Lcd.clear()
            Lcd.drawString("Auth setup failed", 20, 120)
            time.sleep(2)
            return

        # Main loop
        while True:
            M5.update()
            time.sleep(0.1)

    except Exception as e:
        Lcd.clear()
        Lcd.drawString("Error:", 20, 120)
        Lcd.drawString(str(e), 20, 160)
        time.sleep(5)
    
    finally:
        if hasattr(app, 'wlan') and app.wlan:
            app.wlan.active(False)

if __name__ == '__main__':
    main()
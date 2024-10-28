#import aiohttp
#import asyncio
#from typing import Optional
import json
from pathlib import Path
from utils import grey_print
import random
import os

class MemoryManager:
    def __init__(self, assistant_name):  # Currently only has 1 parameter
        print(f"DEBUG: MemoryManager.__init__ called with: {assistant_name}")  # Debug print
        self.assistant_name = assistant_name
        grey_print(f"Initializing MemoryManager for {assistant_name}")
        
        base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        self.memory_dir = os.path.join(base_dir, "CodeMusai", "CoreMemories")
        
        # Create memory directory if it doesn't exist
        os.makedirs(self.memory_dir, exist_ok=True)
        

        # Define memory configurations with their file paths and default states
        self.memory_configs = {
            "self": {
                "enabled": True,
                "file_path": os.path.join(self.memory_dir, "self_memories.json")
            },
            "mom": {
                "enabled": True,
                "file_path": os.path.join(self.memory_dir, "mom_memories.json")
            },
            "dad": {
                "enabled": True,
                "file_path": os.path.join(self.memory_dir, "dad_memories.json")
            },
            "david": {
                "enabled": True,
                "file_path": os.path.join(self.memory_dir, "david_memories.json")
            },
            "anna": {
                "enabled": False,
                "file_path": os.path.join(self.memory_dir, "anna_memories.json")
            },
            "emily": {
                "enabled": False,
                "file_path": os.path.join(self.memory_dir, "emily_memories.json")
            },
            "zoe": {
                "enabled": False,
                "file_path": os.path.join(self.memory_dir, "zoe_memories.json")
            }
        }

    def load_memories_from_json(self, file_path: str) -> str:
        """Load and process memories from JSON file"""
        memories = []
        path = Path(file_path)
        
        if not path.exists():
            grey_print(f"Memory file not found: {file_path}")
            return ""
            
        try:
            with open(path, 'r') as file:
                data = json.load(file)
                
                # Process each memory
                for memory in data:
                    # Check if memory should be included based on probability
                    if random.random() < 1/memory["probability"]:
                        # If there's an alternative, randomly choose between text and alternative
                        if "alternative" in memory and random.random() < 0.5:
                            memories.append(memory["alternative"])
                        else:
                            memories.append(memory["text"])
                            
                # Handle grouped memories (like in Zoe's case)
                if isinstance(data, dict) and "group" in data:
                    if random.random() < 1/data["probability"]:
                        for group_memory in data["memories"]:
                            if random.random() < 1/group_memory["probability"]:
                                if "alternative" in group_memory and random.random() < 0.5:
                                    memories.append(group_memory["alternative"])
                                else:
                                    memories.append(group_memory["text"])
                
                if not memories:
                    grey_print(f"No memories were selected from {file_path}")
                    
                return " ".join(memories)
                
        except json.JSONDecodeError as e:
            grey_print(f"Error parsing JSON from {file_path}: {e}")
            return ""
        except Exception as e:
            grey_print(f"Error loading memories from {file_path}: {e}")
            return ""

    def load_memory(self, memory_type: str) -> str:
        """Load a specific memory if enabled"""
        if memory_type not in self.memory_configs:
            grey_print(f"Unknown memory type: {memory_type}")
            return ""
            
        config = self.memory_configs[memory_type]
        if not config["enabled"]:
            grey_print(f"Memory type {memory_type} is disabled")
            return ""
            
        return self.load_memories_from_json(config["file_path"])

    def enable_memory(self, memory_type: str):
        """Enable a specific memory type"""
        if memory_type in self.memory_configs:
            self.memory_configs[memory_type]["enabled"] = True
            grey_print(f"Enabled {memory_type} memories")
        else:
            grey_print(f"Cannot enable unknown memory type: {memory_type}")

    def disable_memory(self, memory_type: str):
        """Disable a specific memory type"""
        if memory_type in self.memory_configs:
            self.memory_configs[memory_type]["enabled"] = False
            grey_print(f"Disabled {memory_type} memories")
        else:
            grey_print(f"Cannot disable unknown memory type: {memory_type}")


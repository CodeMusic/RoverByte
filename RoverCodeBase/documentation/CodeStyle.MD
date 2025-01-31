# 🎨 RoverByte Code Style Guide  
**A Neural-Inspired Architecture for AI Development**  

---

## 🧠 Psychology-Based Naming Convention  

### 🏛 Cortex Organization  
The codebase mirrors a **neural architecture**, organizing functionality into specialized cortices:  

| **Cortex Name**       | **Functionality**                                |
|----------------------|------------------------------------------------|
| **PrefrontalCortex**  | Executive function, decision making           |
| **VisualCortex**      | Visual processing and display                 |
| **AuditoryCortex**    | Sound processing                              |
| **SomatosensoryCortex** | Touch and UI interaction                     |
| **PsychicCortex**     | External communication (WiFi, NFC, IR)        |
| **MotorCortex**       | Physical control systems                      |
| **GameCortex**        | Entertainment and engagement                  |
| **CorpusCallosum**    | Cross-cortex communication hub                |

---

## 🔹 Neural Terminology  

### 🔄 **State Management Terms**  
These terms define **how the system processes and interacts with data**:  

- 🟢 **Perception** → Input processing (e.g., sensors, external signals)  
- 🟠 **Cognition** → Decision-making logic & behavioral control  
- 🔵 **Expression** → Output generation (e.g., LED feedback, UI responses)  
- 🔴 **Synesthesia** → Cross-modal processing (e.g., visualizing sound data)  
- 🟣 **Proprioception** → Self-awareness & internal state monitoring  

### 📂 **Data Structures Naming**  
- **SynapticPathways** → Handles communication between different cortices  
- **NeuralTypes** → Defines core data types used across RoverByte  
- **ProtoPerceptions** → Basic sensory data models  
- **ChromaticContext** → Stores context for color-based processing  

---

## ✍️ **Code Formatting Standards**  

### 🔹 **General Guidelines**  
✅ **Indentation**: Use **4 spaces per level** (no tabs).  
✅ **Line Length**: Keep lines **under 100 characters** where possible.  
✅ **Braces**: Use **K&R style** with braces on the same line:  
```cpp
if (condition) {
    execute();
} else {
    handleError();
}
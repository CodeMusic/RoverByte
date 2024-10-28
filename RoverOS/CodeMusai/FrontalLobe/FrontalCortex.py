import random

def UNCERTAIN_1IN(n: float) -> bool:
    """
    Returns True with probability 1/n, False otherwise
    
    Args:
        n (float): The denominator for probability calculation
        
    Returns:
        bool: True with probability 1/n, False with probability (n-1)/n
    """
    return random.random() >= 1/n
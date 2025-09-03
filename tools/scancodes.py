import os
import pandas as pd

"""
    Helper script for translating scancodes of the "Scan Code Set 1" table from https://wiki.osdev.org/PS/2_Keyboard to my own key codes.

    Clears / Sets the 7th bit of the scan codes.
    Input data is expected as a csv with the following structure:

        SCANCODE, KEY_CODE
        SCANCODE, KEY_CODE
        SCANCODE, KEY_CODE
        ...

    and output can be saved as a similar csv.
"""

"""
    Clears the 7th bit of a integer represented as a hexadecimal string.

    @param num A string thats the hexadecimal representation of an integer.
    @returns Hexadecimal string of the integer with its 7th bit cleared.
"""
def clear_bit_7(num: str) -> str:
    return "0x{0:02x}".format(int(num, 16) & ~(1<<7))

"""
    Sets the 7th bit of a integer represented as a hexadecimal string.

    @param num A string thats the hexadecimal representation of an integer.
    @returns Hexadecimal string of the integer with its 7th bit set.    
"""
def set_bit_7(num: str) -> str:
    return "0x{0:02x}".format(int(num, 16) | (1<<7))

if __name__ == "__main__":
    SRC_DIR = os.path.abspath(os.path.dirname(__file__))
    # Read the csv.
    df = pd.read_csv(f"{SRC_DIR}/scancodes.csv")
    print(df)

    # Remove / Clear the 7th bit from the scancodes.
    print("Removing the 7th bit.")
    # df.iloc[:, 0] = df.iloc[:, 0].apply(clear_bit_7)
    df.iloc[:, 0] = df.iloc[:, 0].apply(set_bit_7)
    print(df)

    save = input("Save result? (y/n) ")
    if (save.lower() == 'y'):
        path = input("Where should the result be saved? ")
        df.to_csv(path, sep=',', index=False)

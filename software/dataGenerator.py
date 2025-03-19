import ndef


def generate_nfc_c_table(records, read_only=False):
    """
    Generate a C-style array for an NFC Type 2 Tag storing any NDEF record(s).

    Args:
        records (list): A list of NDEF records (e.g., ndef.UriRecord, ndef.TextRecord).
        read_only (bool): Whether the tag should be read-only.

    Returns:
        str: C initialization table as a formatted string.
    """
    
    # Define the NFC Capability Container (CC bytes)
    magic_comment_readwrite = "0XE1, 0X40, 0X40, 0X00"  # Read/Write enabled
    magic_comment_only_read = "0XE1, 0X40, 0X40, 0X01"   # Read-Only mode

    # Select the correct CC bytes
    magic_comment = magic_comment_only_read if read_only else magic_comment_readwrite

    # Encode all NDEF records
    ndef_payload = b''.join(ndef.message_encoder(records))

    # Compute the length of the NDEF message
    ndef_length = len(ndef_payload)

    # Construct the final NDEF message with Type 2 NFC header
    ndef_message = bytes([0x03, ndef_length]) + ndef_payload + b'\xFE'  # Add NDEF start + end marker

    # Convert to hexadecimal representation
    hex_message = ndef_message.hex().upper()
    
    hex_split = [f"0X{hex_message[i:i+2]}" for i in range(0, len(hex_message), 2)]

    # Format as C array
    c_table = f"{{{magic_comment}, " + ', '.join(hex_split) + "};"
    
    return c_table


# note : Apple cant read Geo data and texts directly

message_as_text_list = ["Test 1",
                 "Test 2 a bit longer",
                 "Test 3 even looooooooooooooooooooooooooonger",
                 "https://www.youtube.com/watch?v=dQw4w9WgXcQ",
                 "spotify:track:4cOdK2wGLETKBW3PvgPWqT",
]

if __name__ == "__main__":
    number_of_messages = len(message_as_text_list)

    messages_lines = []
    name_of_messagearray = []

    for num, message in enumerate(message_as_text_list):
        # add comment, so we know what the message is
        messages_lines.append(f"// \"{message}\"")
        # add the actual message
        messages_lines.append(f"const uint8_t dat{num}[] = "+ generate_nfc_c_table([ndef.UriRecord(message)], read_only=False))
    
    messages_multilinetext = "\n".join(messages_lines)
    
    name_of_messagearray = ", ".join([f"dat{num}" for num in range(number_of_messages)])
    
    header_content = f"""#ifndef NFC_MESSAGES_H
#define NFC_MESSAGES_H
// Warning this file is auto-generated by dataGenerator.py
// Do not edit this file directly or your changes will be lost on a subsequent run.

#include <stdint.h>

// NFC messages 
{messages_multilinetext}

// array of pointer to messages
const uint8_t *nfcMessages[] = {{{name_of_messagearray}}};

// Number of messages
const uint8_t messageNumber = {number_of_messages};

#endif // NFC_MESSAGES_H"""

    with open("software/src/nfc_messages.h", "w") as header_file:
        header_file.write(header_content)
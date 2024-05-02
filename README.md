# Text Processor RSVP

Text Processor RSVP is a console application written in C that reads text from a file and displays it word-by-word using the Rapid Serial Visual Presentation (RSVP) technique. This method is aimed at enhancing reading speed and focus by presenting words sequentially at a controlled rate.

## About RSVP

Rapid Serial Visual Presentation (RSVP) is a method used primarily in cognitive psychology to study attention and perception. It involves displaying information, such as text or images, sequentially in small doses at a high rate. Here are some key aspects of RSVP:

- **Presentation Rate**: In RSVP, stimuli are shown one at a time at a fixed location, typically at rates ranging from 100 milliseconds to a few seconds per item. This rapid presentation helps control attention and isolate the processing time for individual elements.

- **Attentional Blink**: This phenomenon occurs when two targets are shown in a rapid stream of items. Observers often miss the second target if it appears shortly (about 100-500 milliseconds) after the first target, demonstrating the limitations of human attention.

- **Applications**: RSVP is used not only in research but also in practical applications such as reading assistance technologies and user interfaces. It helps in presenting text one word at a time to increase reading speed and efficiency.

- **Advantages and Challenges**: The main advantage of RSVP is its ability to present information compactly and quickly, which is beneficial where screen space is limited or rapid information absorption is needed. However, challenges include potential information overload and the need for careful design to avoid overwhelming the user.

## Features

- **Reading Text from File**: Open and read the contents of a text file.
- **RSVP Display**: Display text in an RSVP style to enhance rapid reading without moving the eyes.
- **User Interaction**: Control the text display speed, pause/resume the presentation, and navigate through sentences.
- **Dynamic Frame Adjustments**: Adjust the number of words per frame and the frame duration during execution based on user input.
- **Sentence Bookmarking**: Keep track of the last displayed sentence when the program exits.


## Control the Display:
-  **Spacebar**: Pause/resume the text display.
-  **Arrow Left/Right**: Navigate back and forth between sentences.
  ### while paused:
-  **Arrow Up/Down**: Increase or decrease the frame duration.
-  **'A' or 'a'**: Toggle automatic continuation of sentences.
-  **Escape Key**: Exit the current display and save the sentence index.
-  **'1', '2', '3'**: Set the number of words displayed per frame to one, two, or three, respectively.

 ## Prerequisites

To run this program, ensure you have a GCC compiler installed on your system. The program is compatible with both Windows and Unix-like operating systems.

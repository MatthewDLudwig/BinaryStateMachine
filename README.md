# BinaryStateMachine

This was a project I created a while back after a class on processing raw UNIX input, and was heavily influenced by the fact that I was researching Function pointers at the time.  I originally hosted it at https://pastebin.com/FVvHieRw to be included on my resume, but looking back years later that was a mistake as I didn't make my ownership obvious and I can't remove it, but oh well.  It's now hosted on github permanently and may be improved upon to further differentiate from the unclaimable original.

The state machine is callback based and looks to match the current read character with a function.  Based on the result of that function (true/false hence binary) the machine performs an action and either stays on the current state, moves to the next one, or resets to the root state.  The next character to come into the machine will then be handled by the `match` function for the newly selected state.  This was used to print text the user was typing including the arrow keys (detection of arrow keys was the goal of the BSM) following these rules:

 - ESC is always the first character to come in if it's an arrow.
 - \[ is always the second if it's an arrow.
 - A, B, C, or D then follows and that combination of 3 chars represents an arrow key.
 - If a non arrow related character is encountered, or encountered at the incorrect state, simply print the character.

In the above case, if ESC was seen first but then followed by something other than a \[, `match_bracket` would return false, the character would be printed (not the ESC), and then the current state would be set to the ROOT_STATE of the machine.  Keep in mind the left node is defined first in addState so `printChar, ROOTSTATE` refers to what should be done if `match_bracket` returns false.  I'm also not sure why I referred to `ROOTSTATE`, `NEXTSTATE`, and `STAYSTATE` as **masks** when they're clearly **commands** to the machine.

    addState(machine, match_bracket, printChar, ROOTSTATE, doNothing, NEXTSTATE);
    
Further examples for this project may be added in the future or the project may be elaborated upon, as it was a fun project.

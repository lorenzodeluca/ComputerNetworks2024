# ComputerNetworks2024

vim(cause nano is too easy):
cheat sheet: https://vim.rtorr.com/

## Preconditions
**Copy the .vimrc file into your home directory on the server using Unix's scp command**.

Remember to change MATRICOLA with your Unipd Student ID.

Login using your SSH credentials.

    scp -O ./.vimrc MATRICOLA@SERVER_IP:/home/MATRICOLA/.vimrc 


## Content of .vimrc
This .vimrc config file allows you to:
- **Use your mouse**: scrolling wheel to move up or down, click and point
- **Move line Up/Down** using Ctrl+Shift+Up/Down
- Press F8 to **Compile** the **C** program **without exiting Vim**
- Press F9 to **Execute** the **C** program **without exiting Vim**
- Auto close brackets

## Other configurations:
- Replace tabs with 3 spaces
- Highlight matching parentheses
- Auto indent on brackets
- Show line number
- Highlight current line
- Search characters as they are entered
- Search is case insesitive if no case letters are entered, but case sensitive if case letters are entered
- Highlight search results


# How to search in VIM:
<details>
<summary>Click to expand!</summary>

<br>

Search is **UNIDIRECTIONAL** but when the search reach one end of the file, pressing **n** continues the search, starting from the other end of the file.

## Search from the current line **forward**/**backwards**

To search forward use /

To search bacward use ?

x es:

    ESC (go into Command mode)

    /query (forward)
    ?query (backward)

    ENTER (to stop writing in the search query)

    (now all search results of the query are highlighted)

    n (to move to the NEXT occurence in the search results)
    N (to move to the PREVIOUS occurence in the search results)

    ESC (to exit Search mode)
</details>


# How to Compile and Execute without exiting VIM:
<details>
<summary>Click to expand!</summary>

To Compile press F8

To Execute press F9

    ESC (go into Command mode)

    F8 (compile shortcut)
    F9 (execute shortcut)

    CTRL+C (to exit compilation/executable) 

    Enter (to re-enter in vim)
</details>



# How to Move current line Up or Down in VIM:
<details>
<summary>Click to expand!</summary>

    ESC (go into Command mode)

    CTRL+SHIFT+PAGE UP  (to move line up)
    CTRL+SHIFT+PAGE DOWN (to move line down)

    i (go into Insert mode)
</details>


# How to Select, Copy/Cut and Paste in VIM:
<details>
<summary>Click to expand!</summary>

    Select with the mouse the text you want to copy
    [ALTERNATIVE
        ESC (go into Command mode)
        V100G (to select from current line to line 100, included, using Visual mode)]

    y (to Copy/yank)
    d (to Cut/delete)

    p (to Paste after the cursor)
</details>

# How to copy from another file in VIM:
<details>
<summary>Click to expand!</summary>

Open the file from which you want to copy in Vim using:

    vi ogFile.c (ogFile is the destination file)

    ESC (go into Command mode)

    :ePATH/file (open 'source' file at Path)

    (select the lines that you want to copy)
    y (copy/yank)

    :q (close the 'source' file)

    vi ogFile.c (open the 'destination' file)

    p (paste the copied lines into the 'destination' file)
</details>



# If you've made an error, CTRL+z is u:
<details>
<summary>Click to expand!</summary>
    
    ESC (go into Command mode)

    u (to Undo)
</details>


# If you've pressed CTRL + s and now the screen is frozen, press CTRL + q (to unfreeze screen)
<details>
<summary>Click to expand!</summary>

    CTRL + s (now screen is frozen)

    (every command that you type when the screen is frozen will be executed, it just won't be displayed in the terminal)

    CTRL + q (to unfreeze the screen)
</details>


----

In questa repo sono contenuti i programmi sviluppati dal professore durante il corso tenuto nell'A.A. 21/22 e riscritti per essere piÃ¹ chiaramente comprensibili e piÃ¹ estensivamente commentati.
* Client web HTTP/1.1
  * Risoluzione degli hostname in IP
  * Parsing degli header
  * Supporta il `Transfer-Encoding: chunked`
* Server web
  * Invio di file statici nella cartella del server
  * `Autorization` richiesta nella cartella `secure` tramite basic HTTP auth í ½í´— [RFC2617](https://datatracker.ietf.org/doc/html/rfc2617)
  * Invio tramite `Transfer-Encoding: chunked`
* Proxy web
  * ModalitÃ  `GET` in chiaro
  * ModalitÃ  `CONNECT` tramite TLS tunnelling
* Socket web server
  > Compilare con: `gcc wsock.c -o wsock -lcrypto`
  * Handshake socket
  * Comunicazione bidirezionale tra server e client

## Standard di riferimento (Standards)

* RFC1945 **HTTP 1.0** í ½í´— [rfc-editor.org](https://www.rfc-editor.org/rfc/rfc1945)
* RFC2616 **HTTP 1.1** í ½í´— [rfc-editor.org](https://www.rfc-editor.org/rfc/rfc2616)
* RFC3875 **Common Gateway Interface (CGI)** í ½í´— [rfc-editor.org](https://www.rfc-editor.org/rfc/rfc3875.html)
* RFC6265 **Cookies** í ½í´— [rfc-editor.org](https://www.rfc-editor.org/rfc/rfc6265)
* RFC6455 **WebSocket** í ½í´— [rfc-editor.org](https://www.rfc-editor.org/rfc/rfc6455.html)



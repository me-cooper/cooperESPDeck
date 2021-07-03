Das DIY ESP Streamdeck soll ein Streamdeck für Arme werden.

Der ESP dient in dem Fall als Server und Receiver und wird über USB an dem PC angesteckt. 
Über das Smartphone oder Tablet kann man den Server aufrufen und die Deck Buttons betätigen.

Der ESP wiederum kommuniziert via USB-Schnittstelle mit dem Desktop und sendet dementsprechend Befehle (Tastenkombinationen) die man in OBS als Hotkeys verwenden kann.

Kommunikation:

Webbrowser (Websockets) -> ESP8266 -> Serielle Schnittstelle -> Python (Desktop) -> Hotkeys

Dafür kann man in Python die Library `keyboard` verwenden `pip install keyboard`.



----

```cpp
Serial.println("strg+v");
```

Das reicht um am PC den Shortcut auszuführen.

```python
import serial
import keyboard

pairer = serial.Serial("COM4", baudrate=115200, timeout=.1)

while True:
    serialString = str(pairer.readline())
    serialString = serialString[2:][:-5]
            
    if (serialString) and "x0elrlp" not in serialString:
        print(serialString)
        keyboard.press_and_release(serialString)
```



Und ich will es so machen, jeder Button bekommt die selbe Klasse, und die selbe Funktion wird ausgeführt. Und die ID des Elements enthält einfach den Shortcut.

```html
<button class="trigger" id="strg+c">Copy</button>
<button class="trigger" id="strg+v">Paste</button>
```

Und so kann ich allein durch das hinzufügen von Elementen automatisch neue Buttons hinzufügen. Evtl. kann man die Buttons dann sogar in der JSON hinterlegen um nicht mal mehr die HTML Dateien bearbeiten zu müssen. :D Sau nice.

Bräuchte dann glaub nur zwei Arten von Buttons.
1x Buttons 1x Switches

Buttons für einfache Trigger und Switches für `Bildschirm 1 aktiv` oder `Bildschirm 2 aktiv` dass ich direkt auf dem Screen sehe, was gerade aktiv ist oder sowas.


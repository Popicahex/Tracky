# 🤖 TRACKY – Robot Curier și Explorator Autonom

**Tracky** este un robot multifuncțional proiectat pentru transportul de obiecte mici și explorarea în medii diverse. Este capabil să funcționeze autonom sau manual, fiind ideal pentru aplicații educaționale, recreative sau prototipare tehnologică.

![image](https://github.com/user-attachments/assets/206dab6c-8415-44bb-a08f-70aee80e0e43)

![image1](https://github.com/user-attachments/assets/d985302e-6464-45b4-9d1e-f0aeb73db4c6)

🧑‍💻 Autor
Nume: Popescu Andrei George Constantin
Colegiul Naţional de Informatică "Carmen Sylva" din Petroşani
Clasa: X-A

Localitate: Petroșani, Hunedoara

Proiect realizat integral individual

## 📌 Descriere generală

Tracky este un robot autonom ce:
- Urmărește sursele de lumină pentru a maximiza colectarea energiei prin panou solar.
- Evită obstacolele folosind senzorul ultrasonic.
- Este controlabil prin telecomandă IR sau controller PS2.
- Poate captura și salva imagini pe card SD prin două ESP32-CAM.
- Dispune de un braț robotic funcțional cu 3 servo motoare.

## 🎯 Obiective

- Transport obiecte mici în spații interioare și exterioare.
- Navigație autonomă cu optimizare energetică.
- Control dual: manual (IR/PS2) și automat.
- Utilizare didactică în ateliere STEM.

## ⚙️ Funcționalități principale

| Funcție                       | Descriere                                                       |
|------------------------------|------------------------------------------------------------------|
| 🚗 Navigare autonomă         | Pe baza senzorilor LDR și ultrasonic                            |
| 🔋 Alimentare solară         | Panou solar pentru extinderea autonomiei                        |
| 🎮 Control manual            | IR remote & controller PS2                                      |
| 🦾 Braț robotic              | Cu 3 servo motoare pentru manipulare obiecte                    |
| 📸 Captură imagini           | ESP32-CAM pentru salvare de imagini pe card SD                  |
| 🧠 Mod mixt                  | Comutare între mod automat și manual printr-un buton            |

## 🧩 Componente utilizate

- **Microcontroler:** Arduino UNO R3
- **Camere:** 2x ESP32-CAM cu OV2640
- **Senzori:** HC-SR04 (ultrasonic), 2x LDR (luminozitate)
- **Motoare:** 2x DC + 3x Servo LFD-01M
- **Braț robotic:** 3 grade de libertate
- **Alte module:** IR receiver, PS2 receiver, buzzer, AMS1117 regulator, modul powerbank

## 🛠️ Diagrama electrică

Diagrama completă este realizată în Cirkit Designer / Tinkercad și include toate conexiunile senzorilor, motoarelor, ESP32-CAM și controlerele.

![circuit_image](https://github.com/user-attachments/assets/2b444f90-ca13-410a-938b-fc5982bc18f9)

![Tracky](https://github.com/user-attachments/assets/a447c9d4-7c40-4f42-b3dd-8106f8ed4e5d)

## 🖼️ Arhitectura software

- **Mod Autonom:** Urmărire lumină și evitare obstacole.
- **Mod Manual:** Comenzi IR sau PS2.
- **Mod Mixt:** Comutare prin buton.
- **Captură imagine:** Cod dedicat pentru ESP32-CAM pentru salvare `.pgm`.

⚠️ Codul nu va funcționa fără instalarea bibliotecilor: IR_remote.h, keymap.h, Servo.h și PS2X_lib.h.

⚠️ Pentru ca acest cod să funcționeze, bibliotecile IR_remote.h, keymap.h, Servo.h și PS2X_lib.h trebuie să fie instalate și plasate în același folder cu fișierul sursă.



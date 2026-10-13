<div align="center">

# MiniMacro Lite

**Компактный настраиваемый макропад на ESP32-C3 SuperMini и KY-040**

BLE HID · Wi-Fi Web UI · Custom actions

[Русский](#русский) · [English](#english)

</div>

---

# Русский

MiniMacro Lite превращает **ESP32-C3 SuperMini** и энкодер **KY-040** в компактный макропад. Назначай действия поворотам и нажатиям энкодера через локальную веб-панель — без отдельного приложения.

## Возможности

- **Bluetooth Low Energy HID** — устройство представляется совместимым компьютерам и телефонам как клавиатура с именем `MiniMacro Lite`.
- **Локальная веб-панель** — управление через Wi-Fi-точку доступа платы.
- **7 событий энкодера** — поворот влево/вправо, короткое и двойное нажатия, долгое нажатие, поворот при удержании кнопки в обоих направлениях.
- **Гибкие действия** — медиа-команды, сочетания клавиш, ввод текста или отключённое действие.
- **Сохранение настроек** — назначения сохраняются во flash-памяти и остаются после перезапуска.
- **Адаптивный интерфейс** — можно открыть панель с телефона или компьютера.
- **Тестирование действий** — проверяй назначение прямо из панели.

## Подключение KY-040

| KY-040 | ESP32-C3 SuperMini |
| --- | --- |
| `VCC` / `+` | `3V3` |
| `GND` | `GND` |
| `CLK` | `GPIO8` |
| `DT` | `GPIO10` |
| `SW` | `GPIO9` |

Питай энкодер от **3,3 В** и обязательно соедини `GND` модуля с `GND` платы.

> **Важно:** GPIO8 и GPIO9 у ESP32-C3 относятся к strapping pins. Не удерживай кнопку энкодера нажатой при включении или сбросе платы. Если это мешает загрузке, перенеси сигналы на другие подходящие GPIO и обнови `PIN_ENC_CLK` / `PIN_ENC_SW` в исходнике.

## Установка и прошивка

### Требования

- Arduino IDE 2.x или PlatformIO.
- Пакет плат **esp32 by Espressif Systems 3.x**.
- Библиотеки **NimBLE-Arduino** (2.3.8 или новее) и **HijelHID / HijelHID_BLEKeyboard**.

### Arduino IDE

1. Открой `MiniMacro_Lite.ino`.
2. Выбери плату `ESP32C3 Dev Module`.
3. Установи `USB CDC On Boot: Enabled`.
4. Подключи ESP32-C3 кабелем с поддержкой передачи данных и выбери нужный порт.
5. Нажми **Upload**. Если загрузка не начинается, переведи плату в bootloader-режим и повтори попытку.
6. При необходимости открой Serial Monitor на скорости `115200` бод.

В репозитории также есть `platformio.ini`; PlatformIO может установить объявленные зависимости автоматически при наличии доступа к интернету.

> **Статус проверки:** исходники подготовлены, но сборка и тестирование на физической плате не подтверждены. Не считай проект проверенной готовой прошивкой до успешной компиляции и теста на своём устройстве.

## Первый запуск

1. Подключись к Wi-Fi-точке доступа **`MiniMacro Lite`**.
2. Пароль: **`macroC3lite`**. Если система предупредит, что у сети нет интернета, останься подключённым к ней.
3. Открой в браузере **http://192.168.4.1**.
4. В настройках Bluetooth найди и подключи **`MiniMacro Lite`** как клавиатуру.
5. Настрой действия в веб-панели и сохрани изменения.
6. Для проверки сочетаний открой подходящее приложение и нажми **Test** у нужного действия.

Wi-Fi используется для настройки, а BLE — для отправки клавиатурных и медиа-команд. Они рассчитаны на одновременную работу.

## Назначения по умолчанию

| Событие | Действие |
| --- | --- |
| Поворот влево | Громкость − |
| Поворот вправо | Громкость + |
| Короткое нажатие | Play / Pause |
| Двойное нажатие | Следующий трек |
| Долгое нажатие | Без звука |
| Влево + удержание | `CTRL+LEFT` |
| Вправо + удержание | `CTRL+RIGHT` |

Примеры сочетаний: `CTRL+C`, `CTRL+V`, `CTRL+SHIFT+S`, `ALT+TAB`, `WIN+R`, `F5`, `ENTER`, `ESC`, `LEFT`, `RIGHT`, `UP`, `DOWN`, `SPACE`, `BACKSPACE`, `DELETE`.

Режим ввода текста ориентирован на ASCII/латиницу. Фактический ввод зависит от раскладки и устройства-получателя.

## Решение проблем

- **Не открывается панель:** убедись, что подключён к Wi-Fi `MiniMacro Lite`, и введи адрес `http://192.168.4.1` вручную.
- **BLE видно, но команды не работают:** подключи устройство в Bluetooth-настройках как клавиатуру и проверь ввод в подходящем приложении.
- **Направление вращения перепутано:** поменяй местами назначения в веб-панели или логику направлений в исходнике.
- **Пропуски или двойные срабатывания:** проверь питание 3,3 В, общую землю и качество контактов энкодера.
- **Плата не загружается:** не нажимай кнопку энкодера во время включения; см. примечание о GPIO8/GPIO9 выше.

## Зависимости и лицензии

Проект использует сторонние библиотеки, лицензии которых необходимо соблюдать при распространении. Перед публикацией собранной прошивки проверь лицензии и условия используемых зависимостей. Исходники MiniMacro Lite предоставлены для изучения, настройки и экспериментов; условия дальнейшего распространения следует определить отдельно.

---

# English

MiniMacro Lite turns an **ESP32-C3 SuperMini** and a **KY-040 rotary encoder** into a compact, configurable macropad. Assign actions to encoder turns and button presses through a local web interface — no companion app required.

## Features

- **Bluetooth Low Energy HID** — appears to compatible computers and phones as a keyboard named `MiniMacro Lite`.
- **Local web interface** — configure the device through its Wi-Fi access point.
- **7 encoder events** — turn left/right, single click, double click, long press, and turn left/right while holding the button.
- **Custom actions** — media controls, keyboard shortcuts, text input, or no action.
- **Persistent settings** — assignments are stored in flash and survive reboots.
- **Responsive UI** — use the control panel from a phone or computer.
- **Action testing** — test an assignment directly from the web panel.

## KY-040 wiring

| KY-040 pin | ESP32-C3 SuperMini |
| --- | --- |
| `VCC` / `+` | `3V3` |
| `GND` | `GND` |
| `CLK` | `GPIO8` |
| `DT` | `GPIO10` |
| `SW` | `GPIO9` |

Power the encoder from **3.3 V** and connect the module's `GND` to the board's `GND`.

> **Important:** GPIO8 and GPIO9 are ESP32-C3 strapping pins. Do not hold the encoder button while powering on or resetting the board. If this interferes with booting, move the signals to other suitable GPIOs and update `PIN_ENC_CLK` / `PIN_ENC_SW` in the source.

## Build and flash

### Requirements

- Arduino IDE 2.x or PlatformIO.
- **esp32 by Espressif Systems 3.x** board package.
- **NimBLE-Arduino** (2.3.8 or newer) and **HijelHID / HijelHID_BLEKeyboard** libraries.

### Arduino IDE

1. Open `MiniMacro_Lite.ino`.
2. Select `ESP32C3 Dev Module` as the board.
3. Set `USB CDC On Boot: Enabled`.
4. Connect the ESP32-C3 with a data-capable USB cable and select its port.
5. Click **Upload**. If uploading does not start, enter the board's bootloader mode and try again.
6. If needed, open Serial Monitor at `115200` baud.

A `platformio.ini` file is also included. PlatformIO can install the declared dependencies automatically when internet access is available.

> **Verification status:** the source files are prepared, but compilation and testing on physical hardware have not been confirmed. Treat this as an unverified source project until it builds successfully and is tested on your board.

## First boot

1. Connect to the **`MiniMacro Lite`** Wi-Fi access point.
2. Password: **`macroC3lite`**. If prompted that the network has no internet access, stay connected.
3. Open **http://192.168.4.1** in a browser.
4. Open Bluetooth settings and pair with **`MiniMacro Lite`** as a keyboard.
5. Configure your actions in the web panel and save the changes.
6. To test shortcuts, open a suitable application and use the **Test** button for the desired action.

Wi-Fi is used for configuration, while BLE sends keyboard and media commands. They are intended to operate simultaneously.

## Default mappings

| Event | Default action |
| --- | --- |
| Turn left | Volume down |
| Turn right | Volume up |
| Short press | Play / Pause |
| Double press | Next track |
| Long press | Mute |
| Turn left + hold | `CTRL+LEFT` |
| Turn right + hold | `CTRL+RIGHT` |

Example shortcuts: `CTRL+C`, `CTRL+V`, `CTRL+SHIFT+S`, `ALT+TAB`, `WIN+R`, `F5`, `ENTER`, `ESC`, `LEFT`, `RIGHT`, `UP`, `DOWN`, `SPACE`, `BACKSPACE`, `DELETE`.

Text input is designed for ASCII/Latin characters. Actual output depends on the receiving device and its keyboard layout.

## Troubleshooting

- **The web panel will not load:** make sure you are connected to the `MiniMacro Lite` Wi-Fi network and enter `http://192.168.4.1` manually.
- **BLE is visible, but commands do not work:** pair the device as a keyboard in Bluetooth settings and test it in a suitable application.
- **Rotation direction is reversed:** swap the assignments in the web panel or adjust the direction logic in the source.
- **Missed or duplicate steps:** check the 3.3 V supply, common ground, and encoder contacts.
- **The board will not boot:** do not press the encoder button during startup; see the GPIO8/GPIO9 note above.

## Dependencies and licensing

This project uses third-party libraries whose licenses must be respected when redistributing the software. Review the licenses and terms of all dependencies before distributing a compiled firmware image. MiniMacro Lite source files are provided for learning, configuration, and experimentation; redistribution terms should be specified separately.

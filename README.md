# 🪴 Smart Automated Hydroponics System

Этот проект — интеллектуальная система гидропоники с удалённым управлением. Система контролирует параметры окружающей среды и питательного раствора для оптимального роста растений.

## 🚀 Основные возможности
- 📊 Мониторинг параметров: температура, влажность, освещённость, уровень pH и уровень воды
- 💧 Автоматическое управление поливом и освещением
- 📱 Веб-интерфейс и мобильное приложение для управления
- 🧠 Интеллектуальные алгоритмы регулировки условий роста
- 🌐 Работа через Wi-Fi с возможностью удалённого доступа

## 🧰 Технологии
- **Контроллеры:** ESP32 / Arduino Mega + ESP8266
- **Датчики:** DHT22, pH-метр, уровень воды, световой датчик
- **Сервер:** Java + Spring Boot
- **Фронтенд:** React / HTML + CSS + JS
- **База данных:** MySQL
- **Протоколы:** MQTT / HTTP API

## 📦 Структура проекта
```
smart-hydroponics/
├── firmware/              # Код для микроконтроллеров
├── backend/               # Сервер на Spring Boot
├── frontend/              # Веб-интерфейс управления
├── mobile-app/            # Мобильное приложение (при наличии)
├── sql/                   # Схемы базы данных
└── docs/                  # Документация и схемы подключения
```

## ⚙️ Как работает
1. Датчики собирают данные об окружающей среде и растворе.
2. ESP32 отправляет данные на сервер через Wi-Fi.
3. Сервер сохраняет данные в базу и отображает на веб-интерфейсе.
4. Пользователь может задать параметры (порог pH, влажности и т.д.).
5. Система автоматически включает насосы и освещение при необходимости.

## 💡 Возможности для развития
- Облачное хранилище данных и аналитика роста
- Подключение камер и распознавание состояния растений
- Telegram-бот для уведомлений
- Голосовое управление через Google Assistant

## 👨‍💻 Автор
Керімбек Бекжан Құралбекұлы  
[GitHub](https://github.com/6ecool)

# Deobfuscator

Многофункциональный деобфускатор для C++, C, C#, Python, JavaScript и Java с графическим интерфейсом на ImGui + DirectX 11.

## Возможности

- Поддержка 6 языков: C++, C, C#, Python, JavaScript, Java
- 37 пассов деобфускации
- Многопроходная обработка (до 10 раундов)
- Графический интерфейс на ImGui с подсветкой синтаксиса
- Асинхронная обработка в отдельном потоке
- Буфер обмена и сохранение результата
- Подробное логирование каждого пасса

## Поддерживаемые языки

| Язык | Расширения | Пассов |
|------|------------|--------|
| C++ | .cpp, .h, .hpp, .cc, .cxx | 15 |
| C | .c, .h | 15 |
| C# | .cs | 15 |
| Python | .py | 7 |
| JavaScript | .js, .ts | 5 |
| Java | .java | 10 |

## Пассы

### C / C++ / C#

| Пасс | Что делает |
|------|-----------|
| define-expand | Раскрывает директивы #define |
| xor-strings | Декодирует XOR-строки |
| decoder | Раскодирует escape-последовательности |
| split-strings | Объединяет разбитые строки: "a" "b" -> "ab" |
| char-math | Сворачивает арифметику символов |
| const-fold | Свёртка константных выражений: 2+3 -> 5 |
| junk-ops | Удаляет мусорные операции: a+0 -> a |
| double-negation | Двойное отрицание: -(-x) -> x |
| opaque-predicates | Сворачивает opaque-предикаты: x*x >= 0 -> true |
| identifiers | Переименовывает мусорные идентификаторы |
| rename | Осмысленное переименование: fn_add |
| dead-functions | Удаляет неиспользуемые функции |
| empty-loops | Удаляет пустые циклы |
| collapse-lines | Сжимает строки |
| fake-branch | Убирает ложные ветвления |

### Python

| Пасс | Что делает |
|------|-----------|
| chr-chains | Сворачивает цепочки chr(): chr(72)+chr(105) -> "Hi" |
| python-base64 | Декодирует base64.b64decode("...") |
| python-base64-vars | Base64 через переменные |
| python-xor-multi | Множественный XOR |
| python-xor | XOR-строки в циклах |
| python-dead-funcs | Удаляет мёртвые _dead*-функции |
| reverse-strings | Разворачивает "abc"[::-1] |

### JavaScript

| Пасс | Что делает |
|------|-----------|
| js-atob | Декодирует atob("...") |
| js-charcode | Сворачивает String.fromCharCode(...) |
| js-reverse | Разворачивает .split('').reverse().join('') |
| js-xor | Декодирует XOR-строки |
| js-funcs | Удаляет мёртвые функции |

### Java

| Пасс | Что делает |
|------|-----------|
| java-unicode-escape | Декодирует \uXXXX |
| java-base64 | Сворачивает new String(Base64.getDecoder().decode("...")) |
| java-charcode | Сворачивает new String(new char[]{...}) |
| java-reverse | Разворачивает new StringBuilder("...").reverse().toString() |
| java-string-xor | Сворачивает (byte)(A ^ B) |
| java-const-propagate | Подставляет final-константы |
| java-dead-code | Убирает if (false), разворачивает if (true) |
| java-opaque | Сворачивает opaque-предикаты |
| java-mba | Упрощает MBA: x*1 -> x, x/1 -> x |
| java-identifiers | Переименовывает _0x*-идентификаторы |

## Сборка

Требования:
- CMake 3.15 или выше
- Visual Studio 2022 с поддержкой C++17
- Windows 10 или выше

### Через Visual Studio

1. Открыть папку проекта в VS: File -> Open -> CMake
2. Выбрать конфигурацию x64-Debug или x64-Release
3. Build -> Build All

### Через консоль

```powershell
cmake -B out\build\x64-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out\build\x64-Debug

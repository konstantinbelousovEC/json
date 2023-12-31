# Сборка

Проект может быть собран с помощью _cmake_ ([**CMakeLists.txt**](https://github.com/konstantinbelousovEC/json/blob/main/CMakeLists.txt)) в статическую библиотеку в режимах **debug** и **release**, а также в исполняемый файл с запуском [_тестов_](https://github.com/konstantinbelousovEC/json/blob/main/tests/tests.cpp) из файла **main.cpp**.

## JSON

В файлах [_json.h_](https://github.com/konstantinbelousovEC/json/blob/main/include/json.h) и [_json.cpp_](https://github.com/konstantinbelousovEC/json/blob/main/src/json.cpp) располагается функционал парсинга JSON.

Класс [**Node**](https://github.com/konstantinbelousovEC/json/blob/27c9169b6ce3ed298d9f40e579c3278b79f291f3/include/json.h#L20) приватно наследуется от **std::variant** и хранит значения одного из следующих типов:

- Целые числа типа _int_.
- Вещественные числа типа _double_.
- Строки — тип _std::string_.
- Логический тип _bool_.
- Массивы:

  using Array = std::vector<Node>;

- Словари:

  using Dict = std::map<std::string, Node>;

- _std::nullptr_t_. Используется, чтобы представить значение **null** в JSON документе. Кроме std::nullptr_t можно было бы также использовать тип _std::monostate_, однако _std::nullptr_t_ кажется более подходящим по смыслу для представления **null**.

Следующие методы **Node** сообщают, хранится ли внутри значение некоторого типа:

- _bool is_int() const_;
- _bool is_double() const_; Возвращает _true_, если в **Node** хранится _int_ либо _double_.
- _bool is_pure_double() const_; Возвращает _true_, если в **Node** хранится _double_.
- _bool is_bool() const_;
- _bool is_string() const_;
- _bool is_null() const_;
- _bool is_array() const_;
- _bool is_dict() const_;

Ниже перечислены методы, которые возвращают хранящееся внутри **Node** значение заданного типа. Если внутри содержится значение другого типа, должно выбрасываться исключение _std::logic_error_.

- _int as_int() const_;
- _bool as_bool() const_;
- _double as_double() const_;. Возвращает значение типа _double_, если внутри хранится _double_ либо _int_. В последнем случае возвращается приведённое в _double_ значение.
- _const std::string& as_string() const_;
- _const Array& as_array() const_;
- _const Map& as_dict() const_;

Объекты **Node** можно сравнивать между собой при помощи == и !=. Значения равны, если внутри них значения имеют одинаковый тип и содержимое.

При загрузке невалидных JSON-документов выбрасывается исключение _json::ParsingError_.

При загрузке и сохранении строк поддерживаются следующие escape-последовательности: \n, \r, \\", \t, \\\\.

## JSON Builder

Класс [**json::Builder**](https://github.com/konstantinbelousovEC/json/blob/27c9169b6ce3ed298d9f40e579c3278b79f291f3/include/json-builder.h#L10), позволяет сконструировать JSON-объект, используя цепочки вызовов методов. Этот класс основан на библиотеке JSON, описанной выше.

Методы класса имеют амортизированную линейную сложность относительно размера входных данных. Исключение — дополнительный логарифмический множитель при добавлении в словарь.

Начнём с простого примера — объекта-строки:

    json::Builder{}.value("just a string"s).build()

Это выражение должно быть объектом **json::Node** и содержать указанную строку. Вывести построенный JSON, как и раньше, можно так:

    json::Print(
        json::Document{
            json::Builder{}
            .value("just a string"s)
            .build()
        },
        std::cout
    );

Вывод:

    "just a string"

Более сложный пример демонстрирует все методы builder-класса на более сложном JSON-объекте:

    json::Print(
        json::Document{
                    // Форматирование не имеет формального значения:
                    // это просто цепочка вызовов методов
            json::Builder{}
            .start_dict()
                .key("key1"s).value(123)
                .key("key2"s).value("value2"s)
                .key("key3"s).start_array()
                    .value(456)
                    .start_dict().end_dict()
                    .start_dict()
                        .key(""s).value(nullptr)
                    .end_dict()
                    .value(""s)
                .end_array()
            .end_dict()
            .build()
        },
        std::cout
    );

Вывод:

    {
        "key1": 123,
        "key2": "value2",
        "key3": [
            456,
            {

            },
            {
                "": null
            },
            ""
        ]
    }

Ниже описана семантика методов класса **json::Builder**, и для понимания дан контекст, в котором они вызываются.

- _key(std::string)_. При определении словаря задаёт строковое значение ключа для очередной пары ключ-значение. Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение с помощью метода _value_ или начинать его определение с помощью _start_dict_ или _start_array_.
- _value(Node::Value)_. Задаёт значение, соответствующее ключу при определении словаря, очередной элемент массива или, если вызвать сразу после конструктора **json::Builder**, всё содержимое конструируемого JSON-объекта. Может принимать как простой объект — число или строку — так и целый массив или словарь. Здесь _Node::Value_ — это синоним для базового класса _Node_, шаблона _std::variant_ с набором возможных типов-значений.
- _start_dict()_. Начинает определение сложного значения-словаря. Вызывается в тех же контекстах, что и _value_. Следующим вызовом обязательно должен быть _Key_ или _end_dict_.
- _start_array()_. Начинает определение сложного значения-массива. Вызывается в тех же контекстах, что и _value_. Следующим вызовом обязательно должен быть _end_array_ или любой, задающий новое значение: _value_, _start_dict_ или _start_array_.
- _end_dict()_. Завершает определение сложного значения-словаря. Последним незавершённым вызовом \*start\** должен быть *start_dict\*.
- _end_array()_. Завершает определение сложного значения-массива. Последним незавершённым вызовом _start_\* должен быть _start_array_.
- _build()_. Возвращает объект _json::Node_, содержащий JSON, описанный предыдущими вызовами методов. К этому моменту для каждого \*start\** должен быть вызван соответствующий *end\*\*. При этом сам объект должен быть определён, то есть вызов **json::Builder{}.build()** недопустим.
- Возвращаемое значение каждого метода, кроме _build_, должно быть _Builder&_.

### Обработка ошибок

Некоторые явные ошибки обнаруживаются на этапе компиляции, а не выбрасываются в виде исключений при запуске программы.

Код **json::Builder** не должен компилироваться в следующих ситуациях:

1. Непосредственно после _key_ вызван не _value_, не _start_dict_ и не _start_array_.
2. После вызова _value_, последовавшего за вызовом _key_, вызван не _key_ и не _end_dict_.
3. За вызовом _start_dict_ следует не _key_ и не _end_dict_.
4. За вызовом _start_array_ следует не _value_, не _start_dict_, не _start_array_ и не _end_array_.
5. После вызова _start_array_ и серии _value_ следует не _value_, не _start_dict_, не _start_array_ и не _end_array_.

Примеры кода, которые не должны компилироваться:

    json::Builder{}.start_dict().build();  // правило 3
    json::Builder{}.start_dict().key("1"s).value(1).value(1);  // правило 2
    json::Builder{}.start_dict().key("1"s).key(""s);  // правило 1
    json::Builder{}.start_array().key("1"s);  // правило 4
    json::Builder{}.start_array().end_dict();  // правило 4
    json::Builder{}.start_array().value(1).value(2).end_dict();  // правило 5

Эта задача решается возвратом специальных вспомогательных классов, допускающих определённые наборы методов.

В случае использования методов в неверном контексте код выбросывает исключение типа _std::logic_error_ с понятным сообщением об ошибке.
Это происходит в следующих ситуациях:

- Вызов некорректного метода сразу после создания **json::Builder**;
- Вызов некорректного метода после _end_\*.

## TODO

- Реализовать возможность парсинга и инициализации JSON-документов на этапе компиляции.

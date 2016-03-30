# WeatherDB
Графический интерфейс на Qt для SQLite базы погодных данных

Read [in English](README.md)

## Особенности
- Архитектура модель/представление
  - Модель:
    - [DBModel](model/dbmodel.h) слегка модифицирует `QSqlTableModel`, чтобы передавать цветовые метки из другой таблицы `QSqlTableModel`
    - [AddRowProxyModel](model/addrowproxymodel.h) переписывает `QIdentityProxyModel` на основе `QAbstractProxyModel`, чтобы создать отдельную строку для ввода данных пользователем
    - [FilterProxyModel](model/filterproxymodel.h) наследует `QSortFilterProxyModel` для базовых сортировки и фильтрации
    - [HorizontalProxyModel](model/horizontalproxymodel.h) переписывает `QIdentityProxyModel` на основе `QAbstractProxyModel`, чтобы транспонировать SQL-таблицу для удобства использования
  - Представление:
    - [CheckboxHeader](view/checkboxheader.h) модифицирует `QHeaderView` для добавления чекбоксов поверх заголовочных кнопок
    - [CollapsibleGroupBox](view/collapsiblegroupbox.h) представляет собой адаптацию [QxtGroupBox](https://bitbucket.org/libqxt/libqxt/src/696423b68972fc9edae318558b8ce26dc187cc40/src/widgets/qxtgroupbox.h) из проекта [LibQxt](https://bitbucket.org/libqxt/libqxt/wiki/Home)
    - [FreezeTableWidget](view/dbview.h) реализует [Frozen Column Example](http://doc.qt.io/qt-5/qtwidgets-itemviews-frozencolumn-example.html), чтобы зафиксировать добавленный для ввода данных пользователем столбец
  - Контроллер (не в полной мере отделён от Представления):
    - [DBdelegates](view/dbdelegates.h) содержит несколько делегатов и редакторов, служащих для представления и изменения данных
    - [TabView](view/tabview.h) представляет каждую вкладку UI, создаёт стэк прокси моделей и управляющих элементов для взаимодействия с базой данных. TabView отвечает за сложные изменения таблицы "weather"
    - [Database](database.h) - синглтон, управляющий настройками приложения и базой данных `QSqlDatabase` с таблицами: "weather" (DBModel) и "marks" (`QSqlTableModel`). Database объект отвечает за все операции с таблицей "marks" и её синхронизацией после изменений в таблице "weather"
- Операции, реализованные в `SQLite`: добавить/удалить строку, добавить/удалить столбец, переставить/переименовать строки
- Интерфейс
  - Содержит вкладки, расширяемые опции
  - Подсказки показываются в статусной строке вкладки
  - Строки таблицы можно передвигать для удобства
  - Вертикальное/горизонтальное представление чисел
  - Возможность отмечать цветом любую ячейку таблицы
  - Русский/Английский язык (на основе языка операционной системы)

#### Замечания
  - [AddRowProxyModel](model/addrowproxymodel.h) и [HorizontalProxyModel](model/horizontalproxymodel.h) используют "protected->public" хак для доступа к методу `createIndex()` модели `QAbstractItemModel` для полноты реализации

## Предложения
Функции для реализации
- Сохранение/восстановление порядка строк во вкладке
- Закрепление/сохранение вкладок, восстановление недавних вкладок
- Действия "Отменить"/"Повторить"
- Переключение языка

### Статическая компиляция
1. Установить Qt: <http://www.qt.io/download/> (Qt 5.5.1 и 5.6.0 с MinGW 4.9.2 проверенно работают)
2. Следовать инструкции на сайте <https://wiki.qt.io/Building_a_static_Qt_for_Windows_using_MinGW>

# WeatherDB
Qt-based graphical interface for SQLite weather database  
Versions used: Qt 5.5.1, MinGW 4.9.2, SQLite 3.8.10.2

Прочитать [на русском](README.ru.md)

## Features
- Model/view architecture
  - Model:
    - [DBModel](model/dbmodel.h) offers slight alteration to `QSqlTableModel` to pass color marks from a separate `QSqlTableModel`
    - [AddRowProxyModel](model/addrowproxymodel.h) reimplements `QIdentityProxyModel` from `QAbstractProxyModel` to introduce separate row for user input
    - [FilterProxyModel](model/filterproxymodel.h) inherits `QSortFilterProxyModel` for basic sorting and filtering
    - [HorizontalProxyModel](model/horizontalproxymodel.h) reimplements `QIdentityProxyModel` from `QAbstractProxyModel` to transpose SQL table for ease of use
  - View:
    - [CheckboxHeader](view/checkboxheader.h) modifies `QHeaderView` to add checkboxes on top of the header buttons
    - [CollapsibleGroupBox](view/collapsiblegroupbox.h) is adapted from [LibQxt](https://bitbucket.org/libqxt/libqxt/wiki/Home) project's [QxtGroupBox](https://bitbucket.org/libqxt/libqxt/src/696423b68972fc9edae318558b8ce26dc187cc40/src/widgets/qxtgroupbox.h)
    - [FreezeTableWidget](view/dbview.h) offers adaptation of [Frozen Column Example](http://doc.qt.io/qt-5/qtwidgets-itemviews-frozencolumn-example.html) to freeze column added for user input
  - Controller (is not fully separated from the View):
    - [DBdelegates](view/dbdelegates.h) contains several delegates and editors used to present and edit data
    - [TabView](view/tabview.h) represents each tab of the UI, it introduces stack of proxy models and controls to interact with the database. TabView is responsible for complex alterations to "weather" table
    - [Database](database.h) is a singleton managing application settings, `QSqlDatabase` and its tables: "weather" (DBModel) and "marks" (`QSqlTableModel`). Database object is responsible for all interactions with "marks" table and its synchronization after "weather" table is changed
- Operations implemented with `SQLite`: add/remove row, add/remove column, reorder/rename rows
- Interface
  - Tabbed, extendable
  - Tips shown in the tab's status bar
  - Table rows are movable for convenience
  - Vertical/normal number representation
  - Color marking any cell of the table
  - English/Russian language (based on system locale)

#### Notes
  - [AddRowProxyModel](model/addrowproxymodel.h) and [HorizontalProxyModel](model/horizontalproxymodel.h) use "protected->public" hack to expose `QAbstractItemModel`'s `createIndex()` for completeness of implementation
  - [Qt 5.5.1 bug](https://bugreports.qt.io/browse/QTBUG-50171) encountered and fixed by reimplementing responsible private method and connecting to the new one

## Suggestions
Features to implement
- Save/restore row ordering in the tab
- Lock/save tabs, restore recent tabs
- Undo/redo actions
- Language switching
- Interface cleanup: widget sizes, styling, font scaling

### Static compilation
1. Install Qt: <http://www.qt.io/download/> (Qt 5.5.1 and 5.6.0 with MinGW 4.9.2 were checked to work)
2. Follow the guide on <https://wiki.qt.io/Building_a_static_Qt_for_Windows_using_MinGW>

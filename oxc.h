/*
    MIT License

    Copyright (c) 2025 Vladimir Abramov <abramov7613@yandex.ru>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <cstdint>      // for uint16_t, int8_t, uint8_t
#include <memory>       // for allocator, unique_ptr
#include <optional>     // for optional
#include <span>         // for span
#include <string>       // for string, basic_string
#include <string_view>  // for string_view, basic_string_view
#include <utility>      // for pair
#include <vector>       // for vector
#include <tuple>        // for tuple

/**
 * oxc - oсновное пространство имен библиотеки
 */
namespace oxc {

using oxc_const = const uint16_t;
using Year = std::string;
using Month = int8_t;
using Day = int8_t;
using Weekday = int8_t;

enum class CalendarFormat {
  J, ///< формат календаря: юлианский
  M, ///< формат календаря: ново-юлианский
  G  ///< формат календаря: григорианский
};
constexpr auto Julian = CalendarFormat::J;    ///< формат календаря: юлианский
constexpr auto Milankovic = CalendarFormat::M;///< формат календаря: ново-юлианский
constexpr auto Grigorian = CalendarFormat::G; ///< формат календаря: григорианский
constexpr auto MIN_YEAR_VALUE = 2;            ///< допустимый минимум для числа года

/**
  *  Функция возвращает true для высокосного года
  *
  *  \param [in] y число года
  *  \param [in] fmt выбор типа календаря для вычислений
  */
bool is_leap_year(const Year& y, const CalendarFormat fmt);

/**
  *  Функция возвращает кол-во дней в месяце
  *
  *  \param [in] month число месяца (1 - январь, 2 - февраль и т.д.)
  *  \param [in] leap признак высокосного года
  */
Day month_length(const Month month, const bool leap);

/**
  *  Функция возвращает текстовое представление константы-свойства даты
  *
  *  \param [in] property любая константа из пространства oxc:: (полный список см. в разделе группы)
  */
std::string property_title(oxc_const property);

/**
 * Класс даты. Реализует преобразования между 3-мя календарными системами (григорианский, юлианский, ново-юлианский)
 * по методу Dr. Louis Strous'a - https://aa.quae.nl/en/reken/juliaansedag.html
 * Для числа года используется строковое представление. Конструктор принимающий строковое число года,
 * бросает исключение если строку невозможно преобразовать в целое число произвольной величины
 * или если число (во всех календарных форматах) < MIN_YEAR_VALUE.
 */
class Date {
  class impl;
  std::unique_ptr<impl> pimpl;
public:
  /**
    *  Возвращает название месяца
    *
    *  \param [in] m число месяца (1 - январь, 2 - февраль и т.д.)
    *  \param [in] rp название в род. падеже
    */
  static std::string month_name(Month m, bool rp=true);
  /**
    *  Возвращает сокращенное название месяца
    *
    *  \param [in] m число месяца (1 - январь, 2 - февраль и т.д.)
    */
  static std::string month_short_name(Month m);
  /**
    *  Возвращает название дня недели
    *
    *  \param [in] w число дня недели (0-вс, 1-пн, 2-вт ...)
    */
  static std::string weekday_name(Weekday w);
  /**
    *  Возвращает сокращенное название дня недели
    *
    *  \param [in] w число дня недели (0-вс, 1-пн, 2-вт ...)
    */
  static std::string weekday_short_name(Weekday w);
  /**
   *  Проверка даты на корректность
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] fmt тип календаря для даты
   */
  static bool check(const Year& y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  /**
   *   Перегруженная версия. Отличается только типом параметров.
   */
  static bool check(const unsigned long long y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  /**
    *  Конструктор
    */
  Date();
  /**
    *  Конструктор
    *
    *  \param [in] y число года
    *  \param [in] m число месяца
    *  \param [in] d число дня
    *  \param [in] fmt тип календаря для вх. даты
    */
  Date(const Year& y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  /**
    *  Конструктор
    *
    *  \param [in] y число года
    *  \param [in] m число месяца
    *  \param [in] d число дня
    *  \param [in] fmt тип календаря для вх. даты
    */
  Date(const unsigned long long y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  Date(const Date&);
  Date& operator=(const Date&);
  Date(Date&&);
  Date& operator=(Date&&);
  virtual ~Date();
  bool operator==(const Date&) const;
  bool operator!=(const Date&) const;
  bool operator<(const Date&) const;
  bool operator<=(const Date&) const;
  bool operator>(const Date&) const;
  bool operator>=(const Date&) const;
  /**
    *  Возвращает true если объект не содержит корректной даты
    */
  bool empty() const;
  /**
    *  Возвращает true если объект содержит корректную дату
    */
  bool is_valid() const;
  explicit operator bool() const;
  /**
    *  Извлекает значение года из даты для определенного типа календаря
    *
    *  \param [in] fmt тип календаря
    */
  Year year(const CalendarFormat fmt=Julian) const;
  /**
    *  Извлекает значение месяца из даты для определенного типа календаря
    *
    *  \param [in] fmt тип календаря
    */
  Month month(const CalendarFormat fmt=Julian) const;
  /**
    *  Извлекает число дня (в месяце) для определенного типа календаря
    *
    *  \param [in] fmt тип календаря
    */
  Day day(const CalendarFormat fmt=Julian) const;
   /**
    *  Извлекает день недели для даты. 0-вс, 1-пн, 2-вт, 3-ср, 4-чт, 5-пт, 6-сб.
    */
  Weekday weekday() const;
  /**
    *  Извлекает дату по типу календаря, в формате std::tuple
    *
    *  \param [in] fmt тип календаря
    */
  std::tuple<Year, Month, Day> ymd(const CalendarFormat fmt=Julian) const;
  /**
    *  Возвращает новую дату, увеличенную на кол-во дней от текущей
    *
    *  \param [in] c кол-во дней
    */
  Date inc_by_days(unsigned long long c=1) const;
  /**
    *  Возвращает новую дату, уменьшенную на кол-во дней от текущей
    *
    *  \param [in] c кол-во дней
    */
  Date dec_by_days(unsigned long long c=1) const;
  /**
    *  Обновляет значение даты
    *
    *  \param [in] y число года
    *  \param [in] m число месяца
    *  \param [in] d число дня
    *  \param [in] fmt тип календаря для вх. даты
    */
  bool reset(const Year& y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  /**
   *   Перегруженная версия. Отличается только типом параметров.
   */
  bool reset(const unsigned long long y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  /**
   *  Return string representation of the stored date.<br>
   *  The optional parameter may contain the following format specifiers:<br><ul>
   *    <li>%%% - A literal percent sign (%)
   *    <li>%JY - year number in julian calendar
   *    <li>%GY - year number in grigorian calendar
   *    <li>%MY - year number in milankovic calendar
   *    <li>%Jq - number of month in julian calendar
   *    <li>%Gq - number of month in grigorian calendar
   *    <li>%Mq - number of month in milankovic calendar
   *    <li>%JQ - number of month in julian calendar two digits format
   *    <li>%GQ - number of month in grigorian calendar two digits format
   *    <li>%MQ - number of month in milankovic calendar two digits format
   *    <li>%Jd - day number in julian calendar
   *    <li>%Gd - day number in grigorian calendar
   *    <li>%Md - day number in milankovic calendar
   *    <li>%Jy - last two digits of the year number in julian calendar
   *    <li>%Gy - last two digits of the year number in grigorian calendar
   *    <li>%My - last two digits of the year number in milankovic calendar
   *    <li>%JM - full name of month in julian calendar
   *    <li>%GM - full name of month in grigorian calendar
   *    <li>%MM - full name of month in milankovic calendar
   *    <li>%JF - full name of month in julian calendar (from 1-st face)
   *    <li>%GF - full name of month in grigorian calendar (from 1-st face)
   *    <li>%MF - full name of month in milankovic calendar (from 1-st face)
   *    <li>%Jm - short name of month in julian calendar
   *    <li>%Gm - short name of month in grigorian calendar
   *    <li>%Mm - short name of month in milankovic calendar
   *    <li>%JD - day number in julian calendar in two digits format
   *    <li>%GD - day number in grigorian calendar in two digits format
   *    <li>%MD - day number in milankovic calendar in two digits format
   *    <li>%wd - number of date weekday (sunday=0; monday=1 ...)
   *    <li>%WD - full name of the date weekday
   *    <li>%Wd - short name of date weekday</ul><br>
   *  Each specifier must contain two symbols, except percent.<br>
   *  Unknown format specifiers will be ignored and copied to the output as-is.
   */
  std::string format(std::string fmt = "%Jd %JM %JY г.") const;
};

/**
 * Класс для работы с церковным календарем. Для удобства поиска и календарных вычислений
 * каждая дата может иметь набор свойств (признаков), определенных
 * константами типа oxc_const (полный список см. в разделе группы). Также предусмотрена
 * возможность настроить номера седмиц для расчета отступок / преступок рядовых литургийных
 * чтений (по умолчанию вычисления производится в соответствии с оф. календарем МП РПЦ).
 */
class OrthodoxCalendar {
  class impl;
  std::unique_ptr<impl> pimpl;
public:
  /**
   * класс для определения евангельских / апостольских чтений
   */
  class ApostolEvangelieReadings {
    friend class OrthodoxCalendar;
    /**
     * старшие 4 бита определяют книгу : `1=апостол`, `2=от матфея`, `3=от марка`, `4=от луки`, `5=от иоанна`<br>
     * младшие 12 бит - определяют номер зачала
     */
    uint16_t n;
    /**
     * уточняющий комментарий зачала.
     */
    std::string_view c;
  public:
    ApostolEvangelieReadings() : n{}, c{} {}
    ApostolEvangelieReadings(uint16_t a, std::string_view b) : n(a), c(b) {}
    /**
     * метод возвращает идентификатор богослужебной книги :
     * `1=апостол`, `2=от матфея`, `3=от марка`, `4=от луки`, `5=от иоанна`
     */
    uint16_t book() const { return n>0 ? (n & 0xF) : 0 ; }
    /**
     * метод возвращает номер зачала
     */
    uint16_t zach() const { return n>0 ? (n >> 4) : 0 ; }
    /**
     * метод возвращает комментарий для зачала
     */
    auto comment() const { return c; }
    bool operator==(const ApostolEvangelieReadings&) const = default;
    explicit operator bool() const { return n>0; }
  };
  OrthodoxCalendar();
  OrthodoxCalendar(const OrthodoxCalendar&);
  OrthodoxCalendar& operator=(const OrthodoxCalendar&);
  OrthodoxCalendar(OrthodoxCalendar&&);
  OrthodoxCalendar& operator=(OrthodoxCalendar&&);
  virtual ~OrthodoxCalendar();
  /**
   *  Метод вычисляет дату православной пасхи по ст. ст.
   *
   *  \param [in] year число года по юлианскому календарю
   */
  std::pair<Month, Day> julian_pascha(const Year& year) const;
  /**
   *  Метод вычисляет дату православной пасхи; возвращаемый объект может быть пустым если дата
   *  не найдена (эта вероятность появляется из-за особенностей григорианского и новоюлианского календарей, когда
   *  дата православной пасхи может выпасть на конец декабря месяца, а дата следующей пасхи выпадает
   *  на начало января перепрыгивая через 1 год. Например: 33808 год по григорианскому или новоюлианскому календарю).
   *  Если infmt == Julian, то возвращаемая дата всегда актуальна.
   *
   *  \param [in] year число года
   *  \param [in] infmt тип календаря для числа года
   */
  Date pascha(const Year& year, const CalendarFormat infmt=Julian) const;
  /**
   *  Метод вычисляет кол-во седмиц зимней отступкu литургийных чтений (значения от -5 до 0)
   *
   *  \param [in] year число года юлианского календаря
   */
  int8_t winter_indent(const Year& year) const;
  /**
   *  Метод вычисляет кол-во седмиц осенней отступкu \ преступки литургийных чтений (значения от -2 до 3)
   *
   *  \param [in] year число года юлианского календаря
   */
  int8_t spring_indent(const Year& year) const;
  /**
   *  Метод вычисляет длительность петрова поста в днях (значения от 8 до 42)
   *
   *  \param [in] year число года юлианского календаря
   */
  int8_t apostol_post_length(const Year& year) const;
  /**
   *  Метод вычисляет глас для указанной даты (значения от 1 до 8. для периода от
   *  суб.лазаревой до недели всех святых: значение < 1)
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] infmt тип календаря для даты
   */
  int8_t date_glas(const Year& y, const Month m, const Day d, const CalendarFormat infmt=Julian) const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  int8_t date_glas(const Date& d) const;
  /**
   *  Метод вычисляет календарный номер по пятидесятнице для указанной даты
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] infmt тип календаря для даты
   *  \return для воскр = номер недели. для остальных дней = номер седмицы.
   *    для периода от начала вел.поста до тр.род.субботы = -1
   */
  int8_t date_n50(const Year& y, const Month m, const Day d, const CalendarFormat infmt=Julian) const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  int8_t date_n50(const Date& d) const;
  /**
   *  Метод вычисляет свойства указанной даты и возвращает массив констант из пространства oxc::
   *  (полный список см. в разделе группы). Возвращаемое значение может быть пустым
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] infmt тип календаря для даты
   */
  std::vector<uint16_t> date_properties(const Year& y, const Month m, const Day d,
        const CalendarFormat infmt=Julian) const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  std::vector<uint16_t> date_properties(const Date& d) const;
  /**
   *  Метод вычисляет рядовые литургийные чтения Апостола указанной даты. Праздники не учитываются.
   *  Возвращаемое значение может быть пустым
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] infmt тип календаря для даты
   */
  ApostolEvangelieReadings date_apostol(const Year& y, const Month m, const Day d,
        const CalendarFormat infmt=Julian) const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  ApostolEvangelieReadings date_apostol(const Date& d) const;
  /**
   *  Метод вычисляет рядовые литургийные чтения Евангелия указанной даты. Праздники не учитываются.
   *  Возвращаемое значение может быть пустым
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] infmt тип календаря для даты
   */
  ApostolEvangelieReadings date_evangelie(const Year& y, const Month m, const Day d,
        const CalendarFormat infmt=Julian) const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  ApostolEvangelieReadings date_evangelie(const Date& d) const;
  /**
   *  Метод вычисляет воскресные Евангелия утрени для указанной даты. Возвращаемое значение может быть пустым
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] infmt тип календаря для даты
   */
  ApostolEvangelieReadings resurrect_evangelie(const Year& y, const Month m, const Day d,
        const CalendarFormat infmt=Julian) const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  ApostolEvangelieReadings resurrect_evangelie(const Date& d) const;
  /**
   *  Метод проверяет соответствует ли указанная дата признаку property
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] property любая константа из пространства oxc:: (полный список см. в разделе группы)
   *  \param [in] infmt тип календаря для даты
   */
  bool is_date_of(const Year& y, const Month m, const Day d, oxc_const property,
        const CalendarFormat infmt=Julian) const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  bool is_date_of(const Date& d, oxc_const property) const;
  /**
   *  Метод возвращает первую найденную дату в указанном году, соответствующую параметру property
   *
   *  \param [in] year число года
   *  \param [in] property любая константа из пространства oxc:: (полный список см. в разделе группы)
   *  \param [in] infmt тип календаря для числа года
   */
  Date get_date_with(const Year& year, oxc_const property, const CalendarFormat infmt=Julian) const;
  /**
   *  Метод возвращает первую найденную дату за указанный период, соответствующую параметру property
   *
   *  \param [in] d1 верхняя граница периода времени для поиска (включительно)
   *  \param [in] d2 нижняя граница периода времени для поиска (включительно)
   *  \param [in] property любая константа из пространства oxc:: (полный список см. в разделе группы)
   */
  Date get_date_inperiod_with(const Date& d1, const Date& d2, oxc_const property) const;
  /**
   *  Метод возвращает все даты в указанном году, соответствующие параметру property; или пустой вектор
   *       если ни одна дата не найдена
   *
   *  \param [in] year число года
   *  \param [in] property любая константа из пространства oxc:: (полный список см. в разделе группы)
   *  \param [in] infmt тип календаря для числа года
   */
  std::vector<Date> get_alldates_with(const Year& year, oxc_const property, const CalendarFormat infmt=Julian) const;
  /**
   *  Метод возвращает все даты за указанный период, соответствующие параметру property; или пустой вектор
   *       если ни одна дата не найдена
   *
   *  \param [in] d1 верхняя граница периода времени для поиска (включительно)
   *  \param [in] d2 нижняя граница периода времени для поиска (включительно)
   *  \param [in] property любая константа из пространства oxc:: (полный список см. в разделе группы)
   */
  std::vector<Date> get_alldates_inperiod_with(const Date& d1, const Date& d2, oxc_const property) const;
  /**
   *  Метод возвращает первую найденную дату в указанном году, соответствующую любому из элементов второго параметра
   *
   *  \param [in] year число года
   *  \param [in] properties массив констант из пространства oxc:: (полный список см. в разделе группы)
   *  \param [in] infmt тип календаря для числа года
   */
  Date get_date_withanyof(const Year& year, std::span<oxc_const> properties, const CalendarFormat infmt=Julian) const;
  /**
   *  Метод возвращает первую найденную дату за указанный период, соответствующую
   *  любому из элементов параметра properties
   *
   *  \param [in] d1 верхняя граница периода времени для поиска (включительно)
   *  \param [in] d2 нижняя граница периода времени для поиска (включительно)
   *  \param [in] properties массив констант из пространства oxc:: (полный список см. в разделе группы)
   */
  Date get_date_inperiod_withanyof(const Date& d1, const Date& d2, std::span<oxc_const> properties) const;
  /**
   *  Метод возвращает первую найденную дату в указанном году, соответствующую всем элементам параметра properties
   *
   *  \param [in] year число года
   *  \param [in] properties массив констант из пространства oxc:: (полный список см. в разделе группы)
   *  \param [in] infmt тип календаря для числа года
   */
  Date get_date_withallof(const Year& year, std::span<oxc_const> properties, const CalendarFormat infmt=Julian) const;
  /**
   *  Метод возвращает первую найденную дату за указанный период, соответствующую всем элементам параметра properties
   *
   *  \param [in] d1 верхняя граница периода времени для поиска (включительно)
   *  \param [in] d2 нижняя граница периода времени для поиска (включительно)
   *  \param [in] properties массив констант из пространства oxc:: (полный список см. в разделе группы)
   */
  Date get_date_inperiod_withallof(const Date& d1, const Date& d2, std::span<oxc_const> properties) const;
  /**
   *  Метод возвращает все даты в указанном году, соответствующие любому из элементов параметра properties
   *
   *  \param [in] year число года
   *  \param [in] properties массив констант из пространства oxc:: (полный список см. в разделе группы)
   *  \param [in] infmt тип календаря для числа года
   */
  std::vector<Date> get_alldates_withanyof(const Year& year, std::span<oxc_const> properties,
        const CalendarFormat infmt=Julian) const;
  /**
   *  Метод возвращает все даты за указанный период, соответствующие любому из элементов параметра properties
   *
   *  \param [in] d1 верхняя граница периода времени для поиска (включительно)
   *  \param [in] d2 нижняя граница периода времени для поиска (включительно)
   *  \param [in] properties массив констант из пространства oxc:: (полный список см. в разделе группы)
   */
  std::vector<Date> get_alldates_inperiod_withanyof(const Date& d1, const Date& d2,
        std::span<oxc_const> properties) const;
  /**
   *  Метод возвращает текстовое описание даты.
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] infmt тип календаря для даты
   *  \param [in] datefmt текстовое представление даты ( см. описание метода Date::format )
   */
  std::string get_description_for_date(const Year& y, const Month m, const Day d,
        const CalendarFormat infmt=Julian, std::string datefmt = "%Jd %JM %JY г.") const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  std::string get_description_for_date(const Date& d, std::string datefmt = "%Jd %JM %JY г.") const;
  /**
   *  Метод возвращает текстовое описание нескольких дат.
   *
   *  \param [in] days массив дат
   *  \param [in] datefmt текстовое представление даты ( см. описание метода Date::format )
   *  \param [in] separator строка-разделитель элементов массива
   */
  std::string get_description_for_dates(std::span<const Date> days, std::string datefmt = "%Jd %JM %JY г.",
        const std::string separator="\n") const;
  /**
   *  Метод для установки номера добавочной седмицы зимней отступкu литургийных чтений, при отступке в 1 седмиц.
   *
   *  \param [in] w1 номер 1-й доп. седмицы.
   *  \return true если установка применена; false в противном случае или если вх. параметр некорректен.
   */
  bool set_winter_indent_weeks_1(const uint8_t w1=33);
  /**
   *  Метод для установки номеров добавочных седмиц зимней отступкu литургийных чтений, при отступке в 2 седмиц.
   *
   *  \param [in] w1 номер 1-й доп. седмицы.
   *  \param [in] w2 номер 2-й доп. седмицы.
   *  \return true если установка применена; false в противном случае или если вх. параметр некорректен.
   */
  bool set_winter_indent_weeks_2(const uint8_t w1=32, const uint8_t w2=33);
  /**
   *  Метод для установки номеров добавочных седмиц зимней отступкu литургийных чтений, при отступке в 3 седмиц.
   *
   *  \param [in] w1 номер 1-й доп. седмицы.
   *  \param [in] w2 номер 2-й доп. седмицы.
   *  \param [in] w3 номер 3-й доп. седмицы.
   *  \return true если установка применена; false в противном случае или если вх. параметр некорректен.
   */
  bool set_winter_indent_weeks_3(const uint8_t w1=31, const uint8_t w2=32, const uint8_t w3=33);
  /**
   *  Метод для установки номеров добавочных седмиц зимней отступкu литургийных чтений, при отступке в 4 седмиц.
   *
   *  \param [in] w1 номер 1-й доп. седмицы.
   *  \param [in] w2 номер 2-й доп. седмицы.
   *  \param [in] w3 номер 3-й доп. седмицы.
   *  \param [in] w4 номер 4-й доп. седмицы.
   *  \return true если установка применена; false в противном случае или если вх. параметр некорректен.
   */
  bool set_winter_indent_weeks_4(const uint8_t w1=30, const uint8_t w2=31, const uint8_t w3=32, const uint8_t w4=33);
  /**
   *  Метод для установки номеров добавочных седмиц зимней отступкu литургийных чтений, при отступке в 5 седмиц.
   *
   *  \param [in] w1 номер 1-й доп. седмицы.
   *  \param [in] w2 номер 2-й доп. седмицы.
   *  \param [in] w3 номер 3-й доп. седмицы.
   *  \param [in] w4 номер 4-й доп. седмицы.
   *  \param [in] w5 номер 5-й доп. седмицы.
   *  \return true если установка применена; false в противном случае или если вх. параметр некорректен.
   */
  bool set_winter_indent_weeks_5(const uint8_t w1=30, const uint8_t w2=31, const uint8_t w3=17,
        const uint8_t w4=32, const uint8_t w5=33);
  /**
   *  Метод для установки номеров добавочных седмиц осенней отступкu литургийных чтений.
   *
   *  \param [in] w1 номер 1-й доп. седмицы.
   *  \param [in] w2 номер 2-й доп. седмицы.
   *  \return true если установка применена; false в противном случае или если вх. параметр некорректен.
   */
  bool set_spring_indent_weeks(const uint8_t w1=10, const uint8_t w2=11);
  /**
   *  Метод установки флага - учитывать ли апостол, при вычислении осенней отступкu литургийных чтений.
   */
  void set_spring_indent_apostol(const bool value=false);
  /**
   *  Метод возвращает настройки вычислении зимней / осенней отступкu литургийных чтений
   *  в виде std::pair из вектора и була; вектор содержит 17 элементов:<ul>
   *   <li>первый элемент - номер добавочной седмицы зимней отступкu при отступке в 1 седмиц.
   *   <li>второй и третий - номера добавочных седмиц зимней отступкu при отступке в 2 седмиц.
   *   <li>следующие 3 элемента - номера добавочных седмиц зимней отступкu при отступке в 3 седмиц.
   *   <li>следующие 4 элемента - номера добавочных седмиц зимней отступкu при отступке в 4 седмиц.
   *   <li>следующие 5 элемента - номера добавочных седмиц зимней отступкu при отступке в 5 седмиц.
   *   <li>последние 2 элемента - номера добавочных седмиц осенней отступкu.</ul>
   *  Возвращаемый bool это флаг определяющий учитывать ли апостол, при вычислении осенней отступкu литургийных чтений.
   */
  std::pair<std::vector<uint8_t>, bool> get_options() const;
};

/**
 * \defgroup block1 группа констант 1 - переходящие дни года
 * @{
 *
 */
oxc_const pasha                   = 1   ;///< Светлое Христово Воскресение. ПАСХА.
oxc_const svetlaya1               = 2   ;///< Понедельник Светлой седмицы.
oxc_const svetlaya2               = 3   ;///< Вторник Светлой седмицы.
oxc_const svetlaya3               = 4   ;///< Среда Светлой седмицы.
oxc_const svetlaya4               = 5   ;///< Четверг Светлой седмицы.
oxc_const svetlaya5               = 6   ;///< Пятница Светлой седмицы.
oxc_const svetlaya6               = 7   ;///< Суббота Светлой седмицы.
oxc_const ned2_popashe            = 8   ;///< Неделя 2-я по Пасхе, апостола Фомы́. Антипасха.
oxc_const s2popashe_1             = 9   ;///< Понедельник 2-й седмицы по Пасхе.
oxc_const s2popashe_2             = 10  ;///< Вторник 2-й седмицы по Пасхе. Ра́доница. Поминовение усопших.
oxc_const s2popashe_3             = 11  ;///< Среда 2-й седмицы по Пасхе.
oxc_const s2popashe_4             = 12  ;///< Четверг 2-й седмицы по Пасхе.
oxc_const s2popashe_5             = 13  ;///< Пятница 2-й седмицы по Пасхе.
oxc_const s2popashe_6             = 14  ;///< Суббота 2-й седмицы по Пасхе.
oxc_const ned3_popashe            = 15  ;///< Неделя 3-я по Пасхе, святых жен-мироносиц.
oxc_const s3popashe_1             = 16  ;///< Понедельник 3-й седмицы по Пасхе.
oxc_const s3popashe_2             = 17  ;///< Вторник 3-й седмицы по Пасхе.
oxc_const s3popashe_3             = 18  ;///< Среда 3-й седмицы по Пасхе.
oxc_const s3popashe_4             = 19  ;///< Четверг 3-й седмицы по Пасхе.
oxc_const s3popashe_5             = 20  ;///< Пятница 3-й седмицы по Пасхе.
oxc_const s3popashe_6             = 21  ;///< Суббота 3-й седмицы по Пасхе.
oxc_const ned4_popashe            = 22  ;///< Неделя 4-я по Пасхе, о расслабленном.
oxc_const s4popashe_1             = 23  ;///< Понедельник 4-й седмицы по Пасхе.
oxc_const s4popashe_2             = 24  ;///< Вторник 4-й седмицы по Пасхе.
oxc_const s4popashe_3             = 25  ;///< Среда 4-й седмицы по Пасхе. Преполове́ние Пятидесятницы.
oxc_const s4popashe_4             = 26  ;///< Четверг 4-й седмицы по Пасхе.
oxc_const s4popashe_5             = 27  ;///< Пятница 4-й седмицы по Пасхе.
oxc_const s4popashe_6             = 28  ;///< Суббота 4-й седмицы по Пасхе.
oxc_const ned5_popashe            = 29  ;///< Неделя 5-я по Пасхе, о самаряны́не.
oxc_const s5popashe_1             = 30  ;///< Понедельник 5-й седмицы по Пасхе.
oxc_const s5popashe_2             = 31  ;///< Вторник 5-й седмицы по Пасхе.
oxc_const s5popashe_3             = 32  ;///< Среда 5-й седмицы по Пасхе. Отдание праздника Преполовения Пятидесятницы.
oxc_const s5popashe_4             = 33  ;///< Четверг 5-й седмицы по Пасхе.
oxc_const s5popashe_5             = 34  ;///< Пятница 5-й седмицы по Пасхе.
oxc_const s5popashe_6             = 35  ;///< Суббота 5-й седмицы по Пасхе.
oxc_const ned6_popashe            = 36  ;///< Неделя 6-я по Пасхе, о слепом.
oxc_const s6popashe_1             = 37  ;///< Понедельник 6-й седмицы по Пасхе.
oxc_const s6popashe_2             = 38  ;///< Вторник 6-й седмицы по Пасхе.
oxc_const s6popashe_3             = 39  ;///< Среда 6-й седмицы по Пасхе. Отдание праздника Пасхи. Предпразднство Вознесения.
oxc_const s6popashe_4             = 40  ;///< Четверг 6-й седмицы по Пасхе. Вознесе́ние Госпо́дне.
oxc_const s6popashe_5             = 41  ;///< Пятница 6-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s6popashe_6             = 42  ;///< Суббота 6-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const ned7_popashe            = 43  ;///< Неделя 7-я по Пасхе. Попразднство Вознесения. Святых отцов Первого Вселенского Собора.
oxc_const s7popashe_1             = 44  ;///< Понедельник 7-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s7popashe_2             = 45  ;///< Вторник 7-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s7popashe_3             = 46  ;///< Среда 7-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s7popashe_4             = 47  ;///< Четверг 7-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s7popashe_5             = 48  ;///< Пятница 7-й седмицы по Пасхе. Отдание праздника Вознесения Господня.
oxc_const s7popashe_6             = 49  ;///< Суббота 7-й седмицы по Пасхе. Троицкая родительская суббота.
oxc_const ned8_popashe            = 50  ;///< Неделя 8-я по Пасхе. День Святой Тро́ицы. Пятидеся́тница.
oxc_const s1po50_1                = 51  ;///< Понедельник Пятидесятницы. День Святаго Духа.
oxc_const s1po50_2                = 52  ;///< Вторник Пятидесятницы.
oxc_const s1po50_3                = 53  ;///< Среда Пятидесятницы.
oxc_const s1po50_4                = 54  ;///< Четверг Пятидесятницы.
oxc_const s1po50_5                = 55  ;///< Пятница Пятидесятницы.
oxc_const s1po50_6                = 56  ;///< Суббота Пятидесятницы. Отдание праздника Пятидесятницы.
oxc_const ned1_po50               = 57  ;///< Неделя 1-я по Пятидесятнице, Всех святых.
oxc_const ned2_po50               = 58  ;///< Неделя 2-я по Пятидесятнице, Всех святых, в земле Русской просиявших.
oxc_const ned3_po50               = 59  ;///< Неделя 3-я по Пятидесятнице.
oxc_const ned4_po50               = 60  ;///< Неделя 4-я по Пятидесятнице.
oxc_const sub_pered14sent         = 61  ;///< Суббота пред Воздвижением.
oxc_const ned_pered14sent         = 62  ;///< Неделя пред Воздвижением.
oxc_const sub_po14sent            = 63  ;///< Суббота по Воздвижении.
oxc_const ned_po14sent            = 64  ;///< Неделя по Воздвижении.
oxc_const sobor_otcev7sobora      = 65  ;///< Память святых отцов VII Вселенского Собора.
oxc_const sub_dmitry              = 66  ;///< Димитриевская родительская суббота.
oxc_const ned_praotec             = 67  ;///< Неделя святых пра́отец.
oxc_const sub_peredrojd           = 68  ;///< Суббота пред Рождеством Христовым.
oxc_const ned_peredrojd           = 69  ;///< Неделя пред Рождеством Христовым, святых отец.
oxc_const sub_porojdestve         = 70  ;///< Суббота по Рождестве Христовом.
oxc_const ned_porojdestve         = 71  ;///< Неделя по Рождестве Христовом.
oxc_const ned_mitar_ifaris        = 72  ;///< Неделя о мытаре́ и фарисе́е.
oxc_const ned_obludnom            = 73  ;///< Неделя о блудном сыне.
oxc_const sub_myasopust           = 74  ;///< Суббота мясопу́стная. Вселенская родительская суббота.
oxc_const ned_myasopust           = 75  ;///< Неделя мясопу́стная, о Страшном Суде.
oxc_const sirnaya1                = 76  ;///< Понедельник сырный.
oxc_const sirnaya2                = 77  ;///< Вторник сырный.
oxc_const sirnaya3                = 78  ;///< Среда сырная.
oxc_const sirnaya4                = 79  ;///< Четверг сырный.
oxc_const sirnaya5                = 80  ;///< Пятница сырная.
oxc_const sirnaya6                = 81  ;///< Суббота сырная.
oxc_const ned_siropust            = 82  ;///< Неделя сыропустная. Воспоминание Адамова изгнания. Прощеное воскресенье.
oxc_const vel_post_d1n1           = 83  ;///< Понедельник 1-й седмицы. Начало Великого поста.
oxc_const vel_post_d2n1           = 84  ;///< Вторник 1-й седмицы великого поста.
oxc_const vel_post_d3n1           = 85  ;///< Среда 1-й седмицы великого поста.
oxc_const vel_post_d4n1           = 86  ;///< Четверг 1-й седмицы великого поста.
oxc_const vel_post_d5n1           = 87  ;///< Пятница 1-й седмицы великого поста.
oxc_const vel_post_d6n1           = 88  ;///< Суббота 1-й седмицы великого поста.
oxc_const vel_post_d0n2           = 89  ;///< Неделя 1-я Великого поста. Торжество Православия.
oxc_const vel_post_d1n2           = 90  ;///< Понедельник 2-й седмицы великого поста.
oxc_const vel_post_d2n2           = 91  ;///< Вторник 2-й седмицы великого поста.
oxc_const vel_post_d3n2           = 92  ;///< Среда 2-й седмицы великого поста.
oxc_const vel_post_d4n2           = 93  ;///< Четверг 2-й седмицы великого поста.
oxc_const vel_post_d5n2           = 94  ;///< Пятница 2-й седмицы великого поста.
oxc_const vel_post_d6n2           = 95  ;///< Суббота 2-й седмицы великого поста.
oxc_const vel_post_d0n3           = 96  ;///< Неделя 2-я Великого поста.
oxc_const vel_post_d1n3           = 97  ;///< Понедельник 3-й седмицы великого поста.
oxc_const vel_post_d2n3           = 98  ;///< Вторник 3-й седмицы великого поста.
oxc_const vel_post_d3n3           = 99  ;///< Среда 3-й седмицы великого поста.
oxc_const vel_post_d4n3           = 100 ;///< Четверг 3-й седмицы великого поста.
oxc_const vel_post_d5n3           = 101 ;///< Пятница 3-й седмицы великого поста.
oxc_const vel_post_d6n3           = 102 ;///< Суббота 3-й седмицы великого поста.
oxc_const vel_post_d0n4           = 103 ;///< Неделя 3-я Великого поста, Крестопоклонная.
oxc_const vel_post_d1n4           = 104 ;///< Понедельник 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d2n4           = 105 ;///< Вторник 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d3n4           = 106 ;///< Среда 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d4n4           = 107 ;///< Четверг 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d5n4           = 108 ;///< Пятница 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d6n4           = 109 ;///< Суббота 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d0n5           = 110 ;///< Неделя 4-я Великого поста.
oxc_const vel_post_d1n5           = 111 ;///< Понедельник 5-й седмицы великого поста.
oxc_const vel_post_d2n5           = 112 ;///< Вторник 5-й седмицы великого поста.
oxc_const vel_post_d3n5           = 113 ;///< Среда 5-й седмицы великого поста.
oxc_const vel_post_d4n5           = 114 ;///< Четверг 5-й седмицы великого поста.
oxc_const vel_post_d5n5           = 115 ;///< Пятница 5-й седмицы великого поста.
oxc_const vel_post_d6n5           = 116 ;///< Суббота 5-й седмицы великого поста. Суббота Ака́фиста. Похвала́ Пресвятой Богородицы.
oxc_const vel_post_d0n6           = 117 ;///< Неделя 5-я Великого поста.
oxc_const vel_post_d1n6           = 118 ;///< Понедельник 6-й седмицы великого поста, ва́ий.
oxc_const vel_post_d2n6           = 119 ;///< Вторник 6-й седмицы великого поста, ва́ий.
oxc_const vel_post_d3n6           = 120 ;///< Среда 6-й седмицы великого поста, ва́ий.
oxc_const vel_post_d4n6           = 121 ;///< Четверг 6-й седмицы великого поста, ва́ий.
oxc_const vel_post_d5n6           = 122 ;///< Пятница 6-й седмицы великого поста, ва́ий.
oxc_const vel_post_d6n6           = 123 ;///< Суббота 6-й седмицы великого поста, ва́ий. Лазарева суббота.
oxc_const vel_post_d0n7           = 124 ;///< Неделя ва́ий (цветоно́сная, Вербное воскресенье). Вход Господень в Иерусалим.
oxc_const vel_post_d1n7           = 125 ;///< Страстна́я седмица. Великий Понедельник.
oxc_const vel_post_d2n7           = 126 ;///< Страстна́я седмица. Великий Вторник.
oxc_const vel_post_d3n7           = 127 ;///< Страстна́я седмица. Великая Среда.
oxc_const vel_post_d4n7           = 128 ;///< Страстна́я седмица. Великий Четверг. Воспоминание Тайной Ве́чери.
oxc_const vel_post_d5n7           = 129 ;///< Страстна́я седмица. Великая Пятница.
oxc_const vel_post_d6n7           = 130 ;///< Страстна́я седмица. Великая Суббота.
/** @} */

/**
 * \defgroup block2 группа констант 2 - непереходящие дни года
 * @{
 *
 */
oxc_const m1d1                   = 1001;///< Обре́зание Господне. Свт. Василия Великого, архиеп. Кесари́и Каппадоки́йской.
oxc_const m1d2                   = 1002;///< Предпразднство Богоявления.
oxc_const m1d3                   = 1003;///< Предпразднство Богоявления.
oxc_const m1d4                   = 1004;///< Предпразднство Богоявления.
oxc_const m1d5                   = 1005;///< Предпразднство Богоявления. На́вечерие Богоявления (Крещенский сочельник). День постный.
oxc_const m1d6                   = 1006;///< Святое Богоявле́ние. Крещение Господа Бога и Спаса нашего Иисуса Христа.
oxc_const m1d7                   = 1007;///< Попразднство Богоявления.
oxc_const m1d8                   = 1008;///< Попразднство Богоявления.
oxc_const m1d9                   = 1009;///< Попразднство Богоявления.
oxc_const m1d10                  = 1010;///< Попразднство Богоявления.
oxc_const m1d11                  = 1011;///< Попразднство Богоявления.
oxc_const m1d12                  = 1012;///< Попразднство Богоявления.
oxc_const m1d13                  = 1013;///< Попразднство Богоявления.
oxc_const m1d14                  = 1014;///< Отдание праздника Богоявления.
oxc_const m3d25                  = 1015;///< Благове́щение Пресвято́й Богоро́дицы.
oxc_const m6d24                  = 1016;///< Рождество́ честно́го сла́вного Проро́ка, Предте́чи и Крести́теля Госпо́дня Иоа́нна.
oxc_const m6d25                  = 1017;///< Отдание праздника рождества Предте́чи и Крести́теля Госпо́дня Иоа́нна.
oxc_const m6d29                  = 1018;///< Славных и всехва́льных первоверхо́вных апостолов Петра и Павла.
oxc_const m8d5                   = 1019;///< Предпразднство Преображения Господня.
oxc_const m8d6                   = 1020;///< Преображение Господа Бога и Спаса нашего Иисуса Христа.
oxc_const m8d7                   = 1021;///< Попразднство Преображения Господня.
oxc_const m8d8                   = 1022;///< Попразднство Преображения Господня.
oxc_const m8d9                   = 1023;///< Попразднство Преображения Господня.
oxc_const m8d10                  = 1024;///< Попразднство Преображения Господня.
oxc_const m8d11                  = 1025;///< Попразднство Преображения Господня.
oxc_const m8d12                  = 1026;///< Попразднство Преображения Господня.
oxc_const m8d13                  = 1027;///< Отдание праздника Преображения Господня.
oxc_const m8d14                  = 1028;///< Предпразднство Успения Пресвятой Богородицы.
oxc_const m8d15                  = 1029;///< Успе́ние Пресвятой Владычицы нашей Богородицы и Приснодевы Марии.
oxc_const m8d16                  = 1030;///< Попразднство Успения Пресвятой Богородицы.
oxc_const m8d17                  = 1031;///< Попразднство Успения Пресвятой Богородицы.
oxc_const m8d18                  = 1032;///< Попразднство Успения Пресвятой Богородицы.
oxc_const m8d19                  = 1033;///< Попразднство Успения Пресвятой Богородицы.
oxc_const m8d20                  = 1034;///< Попразднство Успения Пресвятой Богородицы.
oxc_const m8d21                  = 1035;///< Попразднство Успения Пресвятой Богородицы.
oxc_const m8d22                  = 1036;///< Попразднство Успения Пресвятой Богородицы.
oxc_const m8d23                  = 1037;///< Отдание праздника Успения Пресвятой Богородицы.
oxc_const m9d7                   = 1038;///< Предпразднство Рождества Пресвятой Богородицы.
oxc_const m9d8                   = 1039;///< Рождество Пресвятой Владычицы нашей Богородицы и Приснодевы Марии.
oxc_const m9d9                   = 1040;///< Попразднство Рождества Пресвятой Богородицы.
oxc_const m9d10                  = 1041;///< Попразднство Рождества Пресвятой Богородицы.
oxc_const m9d11                  = 1042;///< Попразднство Рождества Пресвятой Богородицы.
oxc_const m9d12                  = 1043;///< Отдание праздника Рождества Пресвятой Богородицы.
oxc_const m9d13                  = 1044;///< Предпразднство Воздви́жения Честно́го и Животворя́щего Креста Господня.
oxc_const m9d14                  = 1045;///< Всеми́рное Воздви́жение Честно́го и Животворя́щего Креста́ Госпо́дня. День постный.
oxc_const m9d15                  = 1046;///< Попразднство Воздвижения Креста.
oxc_const m9d16                  = 1047;///< Попразднство Воздвижения Креста.
oxc_const m9d17                  = 1048;///< Попразднство Воздвижения Креста.
oxc_const m9d18                  = 1049;///< Попразднство Воздвижения Креста.
oxc_const m9d19                  = 1050;///< Попразднство Воздвижения Креста.
oxc_const m9d20                  = 1051;///< Попразднство Воздвижения Креста.
oxc_const m9d21                  = 1052;///< Отдание праздника Воздвижения Животворящего Креста Господня.
oxc_const m8d29                  = 1053;///< Усекновение главы́ Пророка, Предтечи и Крестителя Господня Иоанна. День постный.
oxc_const m10d1                  = 1054;///< Покро́в Пресвятой Владычицы нашей Богородицы и Приснодевы Марии.
oxc_const m11d20                 = 1055;///< Предпразднство Введения (Входа) во храм Пресвятой Богородицы.
oxc_const m11d21                 = 1056;///< Введе́ние (Вход) во храм Пресвятой Владычицы нашей Богородицы и Приснодевы Марии.
oxc_const m11d22                 = 1057;///< Попразднство Введения.
oxc_const m11d23                 = 1058;///< Попразднство Введения.
oxc_const m11d24                 = 1059;///< Попразднство Введения.
oxc_const m11d25                 = 1060;///< Отдание праздника Введения (Входа) во храм Пресвятой Богородицы.
oxc_const m12d20                 = 1061;///< Предпразднство Рождества Христова.
oxc_const m12d21                 = 1062;///< Предпразднство Рождества Христова.
oxc_const m12d22                 = 1063;///< Предпразднство Рождества Христова.
oxc_const m12d23                 = 1064;///< Предпразднство Рождества Христова.
oxc_const m12d24                 = 1065;///< Предпразднство Рождества Христова. На́вечерие Рождества Христова (Рождественский сочельник).
oxc_const m12d25                 = 1066;///< Рождество Господа Бога и Спаса нашего Иисуса Христа.
oxc_const m12d26                 = 1067;///< Попразднство Рождества Христова.
oxc_const m12d27                 = 1068;///< Попразднство Рождества Христова.
oxc_const m12d28                 = 1069;///< Попразднство Рождества Христова.
oxc_const m12d29                 = 1070;///< Попразднство Рождества Христова.
oxc_const m12d30                 = 1071;///< Попразднство Рождества Христова.
oxc_const m12d31                 = 1072;///< Отдание праздника Рождества Христова.
/** @} */

/**
 * \defgroup block3 группа констант 3 - другие дни года
 * @{
 *
*/
oxc_const sub_peredbogoyav       = 2001;///< Суббота перед Богоявлением.
oxc_const ned_peredbogoyav       = 2002;///< Неделя перед Богоявлением.
oxc_const sub_pobogoyav          = 2003;///< Суббота по Богоявлении.
oxc_const ned_pobogoyav          = 2004;///< Неделя по Богоявлении.
oxc_const sobor_novom_rus        = 2005;///< Собор новомучеников и исповедников Церкви Русской.
oxc_const sobor_3sv              = 2006;///< Собор вселенских учителей и святителей Василия Великого, Григория Богослова и Иоанна Златоустого.
oxc_const sretenie_predpr        = 2007;///< Предпразднство Сре́тения Господня.
oxc_const sretenie               = 2008;///< Сре́тение Господа Бога и Спаса нашего Иисуса Христа.
oxc_const sretenie_poprazd1      = 2009;///< День 1-й Попразднства Сретения Господня.
oxc_const sretenie_poprazd2      = 2010;///< День 2-й Попразднства Сретения Господня.
oxc_const sretenie_poprazd3      = 2011;///< День 3-й Попразднства Сретения Господня.
oxc_const sretenie_poprazd4      = 2012;///< День 4-й Попразднства Сретения Господня.
oxc_const sretenie_poprazd5      = 2013;///< День 5-й Попразднства Сретения Господня.
oxc_const sretenie_poprazd6      = 2014;///< День 6-й Попразднства Сретения Господня.
oxc_const sretenie_otdanie       = 2015;///< Отдание праздника Сретения Господня.
oxc_const obret_gl_ioanna12      = 2016;///< Первое и второе Обре́тение главы Иоанна Предтечи.
oxc_const muchenik_40            = 2017;///< Святых сорока́ мучеников, в Севастийском е́зере мучившихся.
oxc_const blag_predprazd         = 2018;///< Предпразднство Благовещения Пресвятой Богородицы.
oxc_const blag_otdanie           = 2019;///< Отдание праздника Благовещения Пресвятой Богородицы.
oxc_const georgia_pob            = 2020;///< Вмч. Гео́ргия Победоно́сца. Мц. царицы Александры.
oxc_const obret_gl_ioanna3       = 2021;///< Третье обре́тение главы Предтечи и Крестителя Господня Иоанна.
oxc_const sobor_otcev_1_6sob     = 2022;///< Память святых отцов шести Вселенских Соборов.
oxc_const feodor_tir             = 2023;///< Вмч. Феодора Тирона (ок. 306) (переходящее празднование).
oxc_const grigor_palam           = 2024;///< Свт. Григория Паламы, архиеп. Фессалонитского (переходящее празднование).
oxc_const ioann_lestv            = 2025;///< Прп. Иоанна Лествичника (переходящее празднование).
oxc_const mari_egipt             = 2026;///< Прп. Марии Египетской (переходящее празднование).
oxc_const sub_porojdestve_r      = 2027;///< Чтения субботы по Рождестве Христовом.
oxc_const ned_porojdestve_r      = 2028;///< Чтения недели по Рождестве Христовом.
oxc_const sub_peredbogoyav_r     = 2029;///< Чтения субботы пред Богоявлением.
oxc_const ned_peredbogoyav_r     = 2030;///< Чтения недели пред Богоявлением.
oxc_const ned_prav_bogootec      = 2031;///< Правв. Иосифа Обручника, Давида царя и Иакова, брата Господня.
/** @} */

/**
 * \defgroup block4 группа констант 4 - типы праздников
 * @{
 *
*/
oxc_const dvana10_per_prazd     = 3001; ///< Двунадесятые переходящие праздники
oxc_const dvana10_nep_prazd     = 3002; ///< Двунадесятые непереходящие праздники
oxc_const vel_prazd             = 3003; ///< Великие праздники
/** @} */

/**
 * \defgroup block5 группа констант 5 - посты и сплошные седмицы
 * @{
 *
*/
oxc_const post_vel         = 4001;///< Великий пост
oxc_const post_petr        = 4002;///< Петров пост
oxc_const post_usp         = 4003;///< Успенский пост
oxc_const post_rojd        = 4004;///< Рождественский пост
oxc_const full7_svyatki    = 4005;///< Сплошная седмица. Святки
oxc_const full7_mitar      = 4006;///< Сплошная седмица. Мытаря и фарисея
oxc_const full7_sirn       = 4007;///< Сплошная седмица. Сырная (Масленица)
oxc_const full7_pasha      = 4008;///< Сплошная седмица. Светлая
oxc_const full7_troica     = 4009;///< Сплошная седмица. Троицкая
/** @} */

/**
 * \defgroup block6 группа констант 6 - переходящие дни празднования икон Богородицы
 * @{
 *
*/
oxc_const mari_icon_01       = 5001; ///< иконы Божией Матери «Акафистная Дионисиатская (Мироточивая)»
oxc_const mari_icon_02       = 5002; ///< иконы Божией Матери «Аз есмь с вами, и никтоже на вы (Леуши́нская)»
oxc_const mari_icon_03       = 5003; ///< иконы Божией Матери «Девпетуровская-Тамбовская»
oxc_const mari_icon_04       = 5004; ///< иконы Божией Матери «Дубенская (Красногорская)»
oxc_const mari_icon_05       = 5005; ///< иконы Божией Матери «Дектоурская (Доктоурская)»
oxc_const mari_icon_06       = 5006; ///< иконы Божией Матери «Живоносный Источник»
oxc_const mari_icon_07       = 5007; ///< иконы Божией Матери «Межеричская (Жизнеподательница)»
oxc_const mari_icon_08       = 5008; ///< иконы Божией Матери «Зна́мение Курская-Коренная»
oxc_const mari_icon_09       = 5009; ///< иконы Божией Матери «Иверская»
oxc_const mari_icon_10       = 5010; ///< иконы Божией Матери «Избавление От Бед Страждущих»
oxc_const mari_icon_11       = 5011; ///< иконы Божией Матери «Кипрская (Стромынская)»
oxc_const mari_icon_12       = 5012; ///< иконы Божией Матери «Кипрская»
oxc_const mari_icon_13       = 5013; ///< иконы Божией Матери «Казанская Коробейниковская»
oxc_const mari_icon_14       = 5014; ///< иконы Божией Матери «Моздокская (Иверская)»
oxc_const mari_icon_15       = 5015; ///< иконы Божией Матери «Марьиногорская»
oxc_const mari_icon_16       = 5016; ///< иконы Божией Матери «Нерушимая Стена»
oxc_const mari_icon_17       = 5017; ///< иконы Божией Матери «Одигитрия Шуйская»
oxc_const mari_icon_18       = 5018; ///< иконы Божией Матери «Прибавление Ума»
oxc_const mari_icon_19       = 5019; ///< иконы Божией Матери «Споручница грешных Корецкая»
oxc_const mari_icon_20       = 5020; ///< иконы Божией Матери «Тупичевская»
oxc_const mari_icon_21       = 5021; ///< иконы Божией Матери «Табынская»
oxc_const mari_icon_22       = 5022; ///< иконы Божией Матери «Умягчение Злых Сердец»
oxc_const mari_icon_23       = 5023; ///< иконы Божией Матери «Умиление Псковско-Печерская»
oxc_const mari_icon_24       = 5024; ///< иконы Божией Матери «Касперовская»
oxc_const mari_icon_25       = 5025; ///< иконы Божией Матери «Челнская»
/** @} */

/**
 * \defgroup block7 группа констант 7 - переходящие дни празднования святых
 * @{
 *
*/
oxc_const sobor_valaam            = 6001;///< Собо́р преподо́бных отце́в, на Валаа́ме просия́вших.
oxc_const varlaam_hut             = 6002;///< Прп. Варлаа́ма Ху́тынского (переходящее празднование).
oxc_const petr_fevron_murom       = 6003;///< Перенесение мощей блгвв. кн. Петра, в иночестве Давида, и кн. Февронии, в иночестве Евфросинии, Муромских чудотворцев.
oxc_const sobor_bessrebren        = 6004;///< Собор всех Бессребреников.
oxc_const sobor_tversk            = 6005;///< Собор Тверских святых.
oxc_const sobor_kuzbas            = 6006;///< Собор Кузбасских святых.
oxc_const pahomii_kensk           = 6007;///< Прп. Пахомия Кенского (XVI) (переходящее празднование).
oxc_const shio_mg                 = 6008;///< Прп.Шио Мгвимского (VI) (Груз.) (переходящее празднование).
oxc_const prep_dav_gar            = 6009;///< Преподобномучеников отцов Давидо-Гареджийских (1616) (Груз.)(переходящее празднование).
oxc_const hristodul               = 6010;///< Мчч. Христодула и Анастасии Патрских, убиенных в Ахаии (1821) (переходящее празднование).
oxc_const iosif_arimaf            = 6011;///< праведных Иосифа Аримафейского и Никодима (переходящее празднование).
oxc_const tamar_gruz              = 6012;///< Блгв. Тамары, царицы Грузинской (переходящее празднование).
oxc_const pm_avraam_bolg          = 6013;///< Перенесение мощей мч. Авраамия Болгарского (1230)(переходящее празднование).
oxc_const tavif                   = 6014;///< Прав. Тавифы (I)(переходящее празднование).
oxc_const much_fereidan           = 6015;///< Мучеников, в долине Ферейдан (Иран) от персов пострадавших (XVII) (Груз.) (переходящее празднование).
oxc_const dodo_gar                = 6016;///< Прп. Додо Гареджийского (Груз.)(623) (переходящее празднование).
oxc_const david_gar               = 6017;///< Прп. Давида Гареджийского (Груз.)(VI) (переходящее празднование).
oxc_const prep_sokolovsk          = 6018;///< Прпп. Тихона, Василия и Никона Соколовских(XVI) (переходящее празднование).
oxc_const arsen_tversk            = 6019;///< Свт.Арсения, еп. Тверского (переходящее празднование).
oxc_const much_lipsiisk           = 6020;///< Прмчч. Неофита, Ионы, Неофита, Ионы и Парфения Липсийских (переходящее празднование).
oxc_const sobor_altai             = 6021;///< Собор Алтайских святых.
oxc_const sobor_afonpr            = 6022;///< Собор всех преподобных и Богоносных отцов, во Святой Горе Афонской просиявших
oxc_const sobor_belorus           = 6023;///< Собор Белорусских святых
oxc_const sobor_vologod           = 6024;///< Собор Вологодских святых
oxc_const sobor_novgorod          = 6025;///< Собор Новгородских святых
oxc_const sobor_pskov             = 6026;///< Собор Псковских святых
oxc_const sobor_piter             = 6027;///< Собор святых Санкт-Петербургской митрополии
oxc_const sobor_udmurt            = 6028;///< Собор святых Удмуртской земли
oxc_const sobor_volgograd         = 6029;///< Собор всех святых, в земле Волгоградской просиявших
oxc_const sobor_ispan             = 6030;///< Собор святых, в земле Испанской и Португальской просиявших
oxc_const sobor_kuban             = 6031;///< Собор святых Кубанской митрополии
oxc_const sobor_chelyab           = 6032;///< Собор святых Челябинской митрополии
oxc_const sobor_mosk              = 6033;///< Собор Московских святых
oxc_const sobor_nnovgor           = 6034;///< Собор святых Нижегородской митрополии
oxc_const sobor_saratov           = 6035;///< Собор Саратовских святых
oxc_const sobor_butov             = 6036;///< Собор новомучеников, в Бутове пострадавших
oxc_const sobor_kazahst           = 6037;///< Собор новомучеников и исповедников Казахстанских
oxc_const sobor_karel             = 6038;///< Собор новомучеников и исповедников земли Карельской
oxc_const sobor_perm              = 6039;///< Собор святых Пермской митрополии
oxc_const sobor_ppech_prep        = 6040;///< Собор преподобных отцов Псково-Печерских
oxc_const sobor_sinai_prep        = 6041;///< Собор преподобных отцов, на Богошественной Горе Синай подвизавшихся
oxc_const sobor_much_holm         = 6042;///< Собор мучеников Холмских и Подляшских
oxc_const sobor_vseh_prep         = 6043;///< Собор всех преподобных отцов, в подвиге просиявших
oxc_const sobor_kpech_prep        = 6044;///< Собор всех преподобных отцов Киево-Печерских
oxc_const sobor_smolensk          = 6045;///< Собор Смоленских святых
oxc_const sobor_alansk            = 6046;///< Собор Аланских святых
oxc_const sobor_german            = 6047;///< Собор святых, в земле Германской просиявших
/** @} */

}// namespace oxc

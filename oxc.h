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
 * Класс даты
 */
class Date {
  class impl;
  std::unique_ptr<impl> pimpl;
public:
  static std::string month_name(Month m, bool rp=true);
  static std::string month_short_name(Month m);
  static std::string weekday_name(Weekday w);
  static std::string weekday_short_name(Weekday w);
  static bool check(const Year& y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  Date();
  Date(const Year& y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  Date(const std::string& cjdn);
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
  bool empty() const;
  bool is_valid() const;
  explicit operator bool() const;
  Year year(const CalendarFormat fmt=Julian) const;
  Month month(const CalendarFormat fmt=Julian) const;
  Day day(const CalendarFormat fmt=Julian) const;
  /**
   *  Возвращает день недели для даты. 0-вс, 1-пн, 2-вт, 3-ср, 4-чт, 5-пт, 6-сб.
   */
  Weekday weekday() const;
  std::optional<std::tuple<Year, Month, Day>> ymd(const CalendarFormat fmt=Julian) const;
  std::string cjdn() const;
  Date inc_by_days(unsigned long long c=1) const;
  Date dec_by_days(unsigned long long c=1) const;
  Date& reset(const Year& y, const Month m, const Day d, const CalendarFormat fmt=Julian);
  /**
   *  Return string representation of the stored date.
   *  The optional parameter may contain the following format specifiers:
   *    %%% - A literal percent sign (%)
   *    %JY - year number in julian calendar
   *    %GY - year number in grigorian calendar
   *    %MY - year number in milankovic calendar
   *    %Jq - number of month in julian calendar
   *    %Gq - number of month in grigorian calendar
   *    %Mq - number of month in milankovic calendar
   *    %JQ - number of month in julian calendar two digits format
   *    %GQ - number of month in grigorian calendar two digits format
   *    %MQ - number of month in milankovic calendar two digits format
   *    %Jd - day number in julian calendar
   *    %Gd - day number in grigorian calendar
   *    %Md - day number in milankovic calendar
   *    %Jy - last two digits of the year number in julian calendar
   *    %Gy - last two digits of the year number in grigorian calendar
   *    %My - last two digits of the year number in milankovic calendar
   *    %JM - full name of month in julian calendar
   *    %GM - full name of month in grigorian calendar
   *    %MM - full name of month in milankovic calendar
   *    %JF - full name of month in julian calendar (from 1-st face)
   *    %GF - full name of month in grigorian calendar (from 1-st face)
   *    %MF - full name of month in milankovic calendar (from 1-st face)
   *    %Jm - short name of month in julian calendar
   *    %Gm - short name of month in grigorian calendar
   *    %Mm - short name of month in milankovic calendar
   *    %JD - day number in julian calendar in two digits format
   *    %GD - day number in grigorian calendar in two digits format
   *    %MD - day number in milankovic calendar in two digits format
   *    %wd - number of date weekday (sunday=0; monday=1 ...)
   *    %WD - full name of the date weekday
   *    %Wd - short name of date weekday
   *  Each specifier must contain two symbols, except percent.
   *  Unknown format specifiers will be ignored and copied to the output as-is.
   */
  std::string format(const std::string& f="%Jd %JM %JY г. по ст.ст.") const;
};

/**
 * Класс для работы с церковным календарем. Реализация использует std::string
 * и библиотеку boost::multiprecision для задания числа года в датах, что дает
 * возможность работать в неограниченно широком диапазоне, но замедляет работу
 * при вычислении очень больших дат. Поэтому все вычисления кэшируются внутри
 * объекта класса. Любой метод принимающий const std::string& для числа года,
 * бросает исключение если строку невозможно преобразовать в большое целое
 * (boost::multiprecision::cpp_int) или если число < 2. Для календарных вычислений
 * в пределах года - каждая дата может иметь набор свойств (признаков), определенных
 * константами типа oxc_const (полный список см. в разделе группы). Также предусмотрена
 * возможность установить номера седмиц для расчета отступок / преступок рядовых литургийных
 * чтений (по умолчанию вычисления производится в соответствии с оф. календарем МП РПЦ),
 * соответствующие методы сбрасывают внутренний кэш объекта класса. Методы для перевода
 * даты из юлианской в григорианскую и обратно не ограничены максимумом, но следует
 * помнить что для больших величин разница может составлять несколько месяцев, а для
 * очень больших - несколько лет.
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
   *  не найдена (эта вероятность появляется из-за особенностей григорианского календаря, когда
   *  дата православной пасхи может выпасть на конец декабря месяца, а дата следующей пасхи выпадает
   *  на начало января перепрыгивая через 1 год. Например: 33808 год по григорианскому календарю).
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
   *  Метод вычисляет длительность петрова поста в днях.
   *
   *  \param [in] year число года юлианского календаря
   */
  int8_t apostol_post_length(const Year& year) const;
  /**
   *  Метод вычисляет глас для указанной даты
   *
   *  \param [in] y число года
   *  \param [in] m число месяца
   *  \param [in] d число дня
   *  \param [in] infmt тип календаря для даты
   *  \return значения от 1 до 8. для периода от суб.лазаревой до недели всех святых: значение < 1
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
   *  Метод вычисляет свойства указанной даты и возвращает массив констант
   *  из пространства oxc:: (полный список см. в разделе группы)
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
   *  Метод вычисляет воскресные Евангелия утрени для указанной даты.
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
   */
  std::string get_description_for_date(const Year& y, const Month m, const Day d,
        const CalendarFormat infmt=Julian) const;
  /**
   *  Перегруженная версия. Отличается только типом параметров.
   */
  std::string get_description_for_date(const Date& d) const;
  /**
   *  Метод возвращает текстовое описание нескольких дат.
   *
   *  \param [in] days массив дат
   *  \param [in] separator строка-разделитель элементов массива
   */
  std::string get_description_for_dates(std::span<const Date> days,  const std::string separator="\n") const;
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
   *
   *  \param [in] value флаг.
   */
  void set_spring_indent_apostol(const bool value=false);
  /**
   *  Метод возвращает настройки вычислении зимней / осенней отступкu литургийных чтений
   *  в виде std::pair из вектора и була. вектор содержит 17 элементов:<ul>
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
oxc_const pasha              = 1   ;///< Светлое Христово Воскресение. ПАСХА.
oxc_const svetlaya1          = 2   ;///< Понедельник Светлой седмицы.
oxc_const svetlaya2          = 3   ;///< Вторник Светлой седмицы. Иверской иконы Божией Матери.
oxc_const svetlaya3          = 4   ;///< Среда Светлой седмицы.
oxc_const svetlaya4          = 5   ;///< Четверг Светлой седмицы.
oxc_const svetlaya5          = 6   ;///< Пятница Светлой седмицы. Последование в честь Пресвятой Богородицы ради Ее «Живоно́сного Исто́чника».
oxc_const svetlaya6          = 7   ;///< Суббота Светлой седмицы.
oxc_const ned2_popashe       = 8   ;///< Неделя 2-я по Пасхе, апостола Фомы́ . Антипасха.
oxc_const s2popashe_1        = 9   ;///< Понедельник 2-й седмицы по Пасхе.
oxc_const s2popashe_2        = 10  ;///< Вторник 2-й седмицы по Пасхе. Ра́доница. Поминовение усопших.
oxc_const s2popashe_3        = 11  ;///< Среда 2-й седмицы по Пасхе.
oxc_const s2popashe_4        = 12  ;///< Четверг 2-й седмицы по Пасхе.
oxc_const s2popashe_5        = 13  ;///< Пятница 2-й седмицы по Пасхе.
oxc_const s2popashe_6        = 14  ;///< Суббота 2-й седмицы по Пасхе.
oxc_const ned3_popashe       = 15  ;///< Неделя 3-я по Пасхе, святых жен-мироносиц. Правв. Марфы и Марии, сестер прав. Лазаря.
oxc_const s3popashe_1        = 16  ;///< Понедельник 3-й седмицы по Пасхе.
oxc_const s3popashe_2        = 17  ;///< Вторник 3-й седмицы по Пасхе.
oxc_const s3popashe_3        = 18  ;///< Среда 3-й седмицы по Пасхе.
oxc_const s3popashe_4        = 19  ;///< Четверг 3-й седмицы по Пасхе.
oxc_const s3popashe_5        = 20  ;///< Пятница 3-й седмицы по Пасхе.
oxc_const s3popashe_6        = 21  ;///< Суббота 3-й седмицы по Пасхе.
oxc_const ned4_popashe       = 22  ;///< Неделя 4-я по Пасхе, о расслабленном.
oxc_const s4popashe_1        = 23  ;///< Понедельник 4-й седмицы по Пасхе.
oxc_const s4popashe_2        = 24  ;///< Вторник 4-й седмицы по Пасхе.
oxc_const s4popashe_3        = 25  ;///< Среда 4-й седмицы по Пасхе. Преполове́ние Пятидесятницы.
oxc_const s4popashe_4        = 26  ;///< Четверг 4-й седмицы по Пасхе.
oxc_const s4popashe_5        = 27  ;///< Пятница 4-й седмицы по Пасхе.
oxc_const s4popashe_6        = 28  ;///< Суббота 4-й седмицы по Пасхе.
oxc_const ned5_popashe       = 29  ;///< Неделя 5-я по Пасхе, о самаряны́не.
oxc_const s5popashe_1        = 30  ;///< Понедельник 5-й седмицы по Пасхе.
oxc_const s5popashe_2        = 31  ;///< Вторник 5-й седмицы по Пасхе.
oxc_const s5popashe_3        = 32  ;///< Среда 5-й седмицы по Пасхе. Отдание праздника Преполовения Пятидесятницы.
oxc_const s5popashe_4        = 33  ;///< Четверг 5-й седмицы по Пасхе.
oxc_const s5popashe_5        = 34  ;///< Пятница 5-й седмицы по Пасхе.
oxc_const s5popashe_6        = 35  ;///< Суббота 5-й седмицы по Пасхе.
oxc_const ned6_popashe       = 36  ;///< Неделя 6-я по Пасхе, о слепом.
oxc_const s6popashe_1        = 37  ;///< Понедельник 6-й седмицы по Пасхе.
oxc_const s6popashe_2        = 38  ;///< Вторник 6-й седмицы по Пасхе.
oxc_const s6popashe_3        = 39  ;///< Среда 6-й седмицы по Пасхе. Отдание праздника Пасхи. Предпразднство Вознесения.
oxc_const s6popashe_4        = 40  ;///< Четверг 6-й седмицы по Пасхе. Вознесе́ние Госпо́дне.
oxc_const s6popashe_5        = 41  ;///< Пятница 6-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s6popashe_6        = 42  ;///< Суббота 6-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const ned7_popashe       = 43  ;///< Неделя 7-я по Пасхе, святых 318 богоносных отцов Первого Вселенского Собора. Попразднство Вознесения.
oxc_const s7popashe_1        = 44  ;///< Понедельник 7-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s7popashe_2        = 45  ;///< Вторник 7-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s7popashe_3        = 46  ;///< Среда 7-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s7popashe_4        = 47  ;///< Четверг 7-й седмицы по Пасхе. Попразднство Вознесения.
oxc_const s7popashe_5        = 48  ;///< Пятница 7-й седмицы по Пасхе. Отдание праздника Вознесения Господня.
oxc_const s7popashe_6        = 49  ;///< Суббота 7-й седмицы по Пасхе. Троицкая родительская суббота.
oxc_const ned8_popashe       = 50  ;///< Неделя 8-я по Пасхе. День Святой Тро́ицы. Пятидеся́тница.
oxc_const s1po50_1           = 51  ;///< Понедельник Пятидесятницы. День Святаго Духа.
oxc_const s1po50_2           = 52  ;///< Вторник Пятидесятницы.
oxc_const s1po50_3           = 53  ;///< Среда Пятидесятницы.
oxc_const s1po50_4           = 54  ;///< Четверг Пятидесятницы.
oxc_const s1po50_5           = 55  ;///< Пятница Пятидесятницы.
oxc_const s1po50_6           = 56  ;///< Суббота Пятидесятницы. Отдание праздника Пятидесятницы.
oxc_const ned1_po50          = 57  ;///< Неделя 1-я по Пятидесятнице, Всех святых.
oxc_const varlaam_hut        = 58  ;///< Прп. Варлаа́ма Ху́тынского (переходящее празднование в 1-ю пятницу Петрова поста).
oxc_const ned2_po50          = 59  ;///< Неделя 2-я по Пятидесятнице, Всех святых, в земле Русской просиявших.
oxc_const ned3_po50          = 60  ;///< Неделя 3-я по Пятидесятнице. Собор всех новоявле́нных мучеников Христовых по взятии Царяграда пострадавших. Собор Новгородских святых. Собор Белорусских святых. Собор святых Санкт-Петербургской митрополии.
oxc_const ned4_po50          = 61  ;///< Неделя 4-я по Пятидесятнице. Собор преподобных отцов Псково-Печерских.
oxc_const sobor_valaam       = 62  ;///< Собор Валаамских святых.
oxc_const petr_fevron_murom  = 63  ;///< Перенесение мощей блгвв. кн. Петра, в иночестве Давида, и кн. Февронии, в иночестве Евфросинии, Муромских чудотворцев.
oxc_const sub_pered14sent    = 64  ;///< Суббота пред Воздвижением.
oxc_const ned_pered14sent    = 65  ;///< Неделя пред Воздвижением.
oxc_const sub_po14sent       = 66  ;///< Суббота по Воздвижении.
oxc_const ned_po14sent       = 67  ;///< Неделя по Воздвижении.
oxc_const sobor_otcev7sobora = 68  ;///< Память святых отцов VII Вселенского Собора.
oxc_const sub_dmitry         = 69  ;///< Димитриевская родительская суббота.
oxc_const sobor_bessrebren   = 70  ;///< Собор всех Бессребреников.
oxc_const ned_praotec        = 71  ;///< Неделя святых пра́отец.
oxc_const sub_peredrojd      = 72  ;///< Суббота пред Рождеством Христовым.
oxc_const ned_peredrojd      = 73  ;///< Неделя пред Рождеством Христовым, святых отец.
oxc_const sub_porojdestve    = 74  ;///< Суббота по Рождестве Христовом.
oxc_const ned_porojdestve    = 75  ;///< Неделя по Рождестве Христовом. Правв. Ио́сифа Обру́чника, Дави́да царя и Иа́кова, брата Господня.
oxc_const ned_mitar_ifaris   = 76  ;///< Неделя о мытаре́ и фарисе́е.
oxc_const ned_obludnom       = 77  ;///< Неделя о блудном сыне.
oxc_const sub_myasopust      = 78  ;///< Суббота мясопу́стная. Вселенская родительская суббота.
oxc_const ned_myasopust      = 79  ;///< Неделя мясопу́стная, о Страшном Суде.
oxc_const sirnaya1           = 80  ;///< Понедельник сырный.
oxc_const sirnaya2           = 81  ;///< Вторник сырный.
oxc_const sirnaya3           = 82  ;///< Среда сырная.
oxc_const sirnaya4           = 83  ;///< Четверг сырный.
oxc_const sirnaya5           = 84  ;///< Пятница сырная.
oxc_const sirnaya6           = 85  ;///< Суббота сырная. Всех преподобных отцов, в подвиге просиявших.
oxc_const ned_siropust       = 86  ;///< Неделя сыропустная. Воспоминание Адамова изгнания. Прощеное воскресенье.
oxc_const vel_post_d1n1      = 87  ;///< Понедельник 1-й седмицы. Начало Великого поста.
oxc_const vel_post_d2n1      = 88  ;///< Вторник 1-й седмицы великого поста.
oxc_const vel_post_d3n1      = 89  ;///< Среда 1-й седмицы великого поста.
oxc_const vel_post_d4n1      = 90  ;///< Четверг 1-й седмицы великого поста.
oxc_const vel_post_d5n1      = 91  ;///< Пятница 1-й седмицы великого поста.
oxc_const vel_post_d6n1      = 92  ;///< Суббота 1-й седмицы великого поста.
oxc_const vel_post_d0n2      = 93  ;///< Неделя 1-я Великого поста. Торжество Православия.
oxc_const vel_post_d1n2      = 94  ;///< Понедельник 2-й седмицы великого поста.
oxc_const vel_post_d2n2      = 95  ;///< Вторник 2-й седмицы великого поста.
oxc_const vel_post_d3n2      = 96  ;///< Среда 2-й седмицы великого поста.
oxc_const vel_post_d4n2      = 97  ;///< Четверг 2-й седмицы великого поста.
oxc_const vel_post_d5n2      = 98  ;///< Пятница 2-й седмицы великого поста.
oxc_const vel_post_d6n2      = 99  ;///< Суббота 2-й седмицы великого поста.
oxc_const vel_post_d0n3      = 100 ;///< Неделя 2-я Великого поста. Свт. Григория Пала́мы, архиеп. Фессалони́тского. Собор преподобных отец Киево-Печерских и всех святых, в Малой России просиявших.
oxc_const vel_post_d1n3      = 101 ;///< Понедельник 3-й седмицы великого поста.
oxc_const vel_post_d2n3      = 102 ;///< Вторник 3-й седмицы великого поста.
oxc_const vel_post_d3n3      = 103 ;///< Среда 3-й седмицы великого поста.
oxc_const vel_post_d4n3      = 104 ;///< Четверг 3-й седмицы великого поста.
oxc_const vel_post_d5n3      = 105 ;///< Пятница 3-й седмицы великого поста.
oxc_const vel_post_d6n3      = 106 ;///< Суббота 3-й седмицы великого поста.
oxc_const vel_post_d0n4      = 107 ;///< Неделя 3-я Великого поста, Крестопоклонная.
oxc_const vel_post_d1n4      = 108 ;///< Понедельник 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d2n4      = 109 ;///< Вторник 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d3n4      = 110 ;///< Среда 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d4n4      = 111 ;///< Четверг 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d5n4      = 112 ;///< Пятница 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d6n4      = 113 ;///< Суббота 4-й седмицы вел. поста, Крестопоклонной.
oxc_const vel_post_d0n5      = 114 ;///< Неделя 4-я Великого поста. Прп. Иоанна Ле́ствичника.
oxc_const vel_post_d1n5      = 115 ;///< Понедельник 5-й седмицы великого поста.
oxc_const vel_post_d2n5      = 116 ;///< Вторник 5-й седмицы великого поста.
oxc_const vel_post_d3n5      = 117 ;///< Среда 5-й седмицы великого поста.
oxc_const vel_post_d4n5      = 118 ;///< Четверг 5-й седмицы великого поста. Четверто́к Великого канона.
oxc_const vel_post_d5n5      = 119 ;///< Пятница 5-й седмицы великого поста.
oxc_const vel_post_d6n5      = 120 ;///< Суббота 5-й седмицы великого поста. Суббота Ака́фиста. Похвала́ Пресвятой Богородицы.
oxc_const vel_post_d0n6      = 121 ;///< Неделя 5-я Великого поста. Прп. Марии Египетской.
oxc_const vel_post_d1n6      = 122 ;///< Понедельник 6-й седмицы великого поста. ва́ий.
oxc_const vel_post_d2n6      = 123 ;///< Вторник 6-й седмицы великого поста. ва́ий.
oxc_const vel_post_d3n6      = 124 ;///< Среда 6-й седмицы великого поста. ва́ий.
oxc_const vel_post_d4n6      = 125 ;///< Четверг 6-й седмицы великого поста. ва́ий.
oxc_const vel_post_d5n6      = 126 ;///< Пятница 6-й седмицы великого поста. ва́ий.
oxc_const vel_post_d6n6      = 127 ;///< Суббота 6-й седмицы великого поста. ва́ий. Лазарева суббота. Воскрешение прав. Лазаря.
oxc_const vel_post_d0n7      = 128 ;///< Неделя ва́ий (цветоно́сная, Вербное воскресенье). Вход Господень в Иерусалим.
oxc_const vel_post_d1n7      = 129 ;///< Страстна́я седмица. Великий Понедельник.
oxc_const vel_post_d2n7      = 130 ;///< Страстна́я седмица. Великий Вторник.
oxc_const vel_post_d3n7      = 131 ;///< Страстна́я седмица. Великая Среда.
oxc_const vel_post_d4n7      = 132 ;///< Страстна́я седмица. Великий Четверг. Воспоминание Тайной Ве́чери.
oxc_const vel_post_d5n7      = 133 ;///< Страстна́я седмица. Великая Пятница.
oxc_const vel_post_d6n7      = 134 ;///< Страстна́я седмица. Великая Суббота.
/** @} */

/**
 * \defgroup block2 группа констант 2 - непереходящие дни года
 * @{
 *
 */
oxc_const m1d1   = 1001  ;///< 1 января. Обре́зание Господне. Свт. Василия Великого, архиеп. Кесари́и Каппадоки́йской.
oxc_const m1d2   = 1002  ;///< 2 января. Предпразднство Богоявления. Прп. Серафи́ма Саро́вского.
oxc_const m1d3   = 1003  ;///< 3 января. Предпразднство Богоявления. Прор. Малахи́и. Мч. Горди́я.
oxc_const m1d4   = 1004  ;///< 4 января. Предпразднство Богоявления. Собор 70-ти апостолов. Прп. Феокти́ста, игумена Куку́ма Сикели́йского.
oxc_const m1d5   = 1005  ;///< 5 января. Предпразднство Богоявления. На́вечерие Богоявления (Крещенский сочельник). Сщмч. Феопе́мпта, еп. Никомиди́йского, и мч. Фео́ны волхва. Прп. Синклитики́и Александрийской. День постный.
oxc_const m1d6   = 1006  ;///< 6 января. Святое Богоявле́ние. Крещение Господа Бога и Спаса нашего Иисуса Христа.
oxc_const m1d7   = 1007  ;///< 7 января. Попразднство Богоявления. Собор Предтечи и Крестителя Господня Иоанна.
oxc_const m1d8   = 1008  ;///< 8 января. Попразднство Богоявления. Прп. Гео́ргия Хозеви́та. Прп. Домни́ки.
oxc_const m1d9   = 1009  ;///< 9 января. Попразднство Богоявления. Мч. Полие́вкта. Свт. Фили́ппа, митр. Московского и всея России, чудотворца.
oxc_const m1d10  = 1010  ;///< 10 января. Попразднство Богоявления. Свт. Григория, еп. Ни́сского. Прп. Дометиа́на, еп. Мелити́нского. Свт. Феофа́на, Затворника Вы́шенского.
oxc_const m1d11  = 1011  ;///< 11 января. Попразднство Богоявления. Прп. Феодо́сия Великого, общих жити́й начальника.
oxc_const m1d12  = 1012  ;///< 12 января. Попразднство Богоявления. Мц. Татиа́ны.
oxc_const m1d13  = 1013  ;///< 13 января. Попразднство Богоявления. Мчч. Ерми́ла и Стратони́ка. Прп. Ирина́рха, затворника Ростовского.
oxc_const m1d14  = 1014  ;///< 14 января. Отдание праздника Богоявления. Св. равноап. Нины, просветительницы Грузии.
oxc_const m3d25  = 1015  ;///< 25 марта. Благовещ́ение Пресвято́й Богоро́дицы.
oxc_const m5d11  = 1016  ;///< 11 мая. Равноапп. Мефо́дия и Кири́лла, учи́телей Слове́нских.
oxc_const m6d24  = 1017  ;///< 24 июня. Рождество́ честно́го сла́вного Проро́ка, Предте́чи и Крести́теля Госпо́дня Иоа́нна.
oxc_const m6d25  = 1018  ;///< 25 июня. Отдание праздника рождества Предте́чи и Крести́теля Госпо́дня Иоа́нна. Прмц. Февро́нии.
oxc_const m6d29  = 1019  ;///< 29 июня. Славных и всехва́льных первоверхо́вных апостолов Петра и Павла.
oxc_const m6d30  = 1020  ;///< 30 июня. Собор славных и всехвальных 12-ти апостолов.
oxc_const m7d15  = 1021  ;///< 15 июля. Равноап. вел. князя Влади́мира, во Святом Крещении Васи́лия.
oxc_const m8d5   = 1022  ;///< 5 августа. Предпразднство Преображения Господня. Мч. Евсигни́я.
oxc_const m8d6   = 1023  ;///< 6 августа. Преображение Господа Бога и Спаса нашего Иисуса Христа.
oxc_const m8d7   = 1024  ;///< 7 августа. Попразднство Преображения Господня. Прмч. Домети́я. Обре́тение моще́й свт. Митрофа́на, еп. Воро́нежского
oxc_const m8d8   = 1025  ;///< 8 августа. Попразднство Преображения Господня. Свт. Емилиа́на исп., еп. Кизи́ческого. Перенесение мощей прпп. Зоси́мы, Савва́тия и Ге́рмана Солове́цких
oxc_const m8d9   = 1026  ;///< 9 августа. Попразднство Преображения Господня. Апостола Матфи́я.
oxc_const m8d10  = 1027  ;///< 10 августа. Попразднство Преображения Господня. Мч. архидиакона Лавре́нтия. Собор новомучеников и исповедников Солове́цких
oxc_const m8d11  = 1028  ;///< 11 августа. Попразднство Преображения Господня. Мч. архидиакона Е́впла.
oxc_const m8d12  = 1029  ;///< 12 августа. Попразднство Преображения Господня. Мчч. Фо́тия и Аники́ты. Прп. Макси́ма Испове́дника
oxc_const m8d13  = 1030  ;///< 13 августа. Отдание праздника Преображения Господня. Свт. Ти́хона, еп. Воро́нежского, Задо́нского, чудотворца.
oxc_const m8d14  = 1031  ;///< 14 августа. Предпразднство Успения Пресвятой Богородицы. Прор. Михе́я. Перенесение мощей прп. Феодо́сия Пече́рского.
oxc_const m8d15  = 1032  ;///< 15 августа. Успе́ние Пресвятой Владычицы нашей Богородицы и Приснодевы Марии.
oxc_const m8d16  = 1033  ;///< 16 августа. Попразднство Успения Пресвятой Богородицы. Перенесение из Еде́ссы в Константино́поль Нерукотворе́нного О́браза (Убру́са) Господа Иисуса Христа.
oxc_const m8d17  = 1034  ;///< 17 августа. Попразднство Успения Пресвятой Богородицы. Мч. Ми́рона.
oxc_const m8d18  = 1035  ;///< 18 августа. Попразднство Успения Пресвятой Богородицы. Мчч. Фло́ра и Ла́вра.
oxc_const m8d19  = 1036  ;///< 19 августа. Попразднство Успения Пресвятой Богородицы. Мч. Андрея Стратила́та и иже с ним. Донской иконы Божией Матери.
oxc_const m8d20  = 1037  ;///< 20 августа. Попразднство Успения Пресвятой Богородицы. Прор. Самуила.
oxc_const m8d21  = 1038  ;///< 21 августа. Попразднство Успения Пресвятой Богородицы. Ап. от 70-ти Фадде́я. Мц. Ва́ссы.
oxc_const m8d22  = 1039  ;///< 22 августа. Попразднство Успения Пресвятой Богородицы. Мч. Агафони́ка и иже с ним. Мч. Лу́ппа
oxc_const m8d23  = 1040  ;///< 23 августа. Отдание праздника Успения Пресвятой Богородицы.
oxc_const m9d7   = 1041  ;///< 7 сентября. Предпразднство Рождества Пресвятой Богородицы. Мч. Созонта.
oxc_const m9d8   = 1042  ;///< 8 сентября. Рождество Пресвятой Владычицы нашей Богородицы и Приснодевы Марии.
oxc_const m9d9   = 1043  ;///< 9 сентября. Попразднство Рождества Пресвятой Богородицы. Праведных Богооте́ц Иоаки́ма и А́нны. Прп. Ио́сифа, игумена Во́лоцкого, чудотворца.
oxc_const m9d10  = 1044  ;///< 10 сентября. Попразднство Рождества Пресвятой Богородицы. Мцц. Минодо́ры, Митродо́ры и Нимфодо́ры.
oxc_const m9d11  = 1045  ;///< 11 сентября. Попразднство Рождества Пресвятой Богородицы. Прп. Силуа́на Афо́нского.
oxc_const m9d12  = 1046  ;///< 12 сентября. Отдание праздника Рождества Пресвятой Богородицы.
oxc_const m9d13  = 1047  ;///< 13 сентября. Предпразднство Воздви́жения Честно́го и Животворя́щего Креста Господня. Сщмч. Корни́лия со́тника.
oxc_const m9d14  = 1048  ;///< 14 сентября. Всеми́рное Воздви́жение Честно́го и Животворя́щего Креста́ Госпо́дня. День постный.
oxc_const m9d15  = 1049  ;///< 15 сентября. Попразднство Воздвижения Креста. Вмч. Ники́ты.
oxc_const m9d16  = 1050  ;///< 16 сентября. Попразднство Воздвижения Креста. Вмц. Евфи́мии всехва́льной.
oxc_const m9d17  = 1051  ;///< 17 сентября. Попразднство Воздвижения Креста. Мцц. Ве́ры, Наде́жды, Любо́ви и матери их Софи́и.
oxc_const m9d18  = 1052  ;///< 18 сентября. Попразднство Воздвижения Креста. Прп. Евме́ния, еп. Горти́нского.
oxc_const m9d19  = 1053  ;///< 19 сентября. Попразднство Воздвижения Креста. Мчч. Трофи́ма, Савва́тия и Доримедо́нта.
oxc_const m9d20  = 1054  ;///< 20 сентября. Попразднство Воздвижения Креста. Вмч. Евста́фия и иже с ним. Мучеников и исповедников Михаи́ла, кн. Черни́говского, и боля́рина его Фео́дора, чудотворцев.
oxc_const m9d21  = 1055  ;///< 21 сентября. Отдание праздника Воздвижения Животворящего Креста Господня. Обре́тение мощей свт. Дими́трия, митр. Росто́вского.
oxc_const m8d29  = 1056  ;///< 29 августа. Усекновение главы́ Пророка, Предтечи и Крестителя Господня Иоанна. День постный.
oxc_const m10d1  = 1057  ;///< 1 октября. Покро́в Пресвятой Владычицы нашей Богородицы и Приснодевы Марии. Ап. от 70-ти Ана́нии. Прп. Рома́на Сладкопе́вца.
oxc_const m11d20 = 1058  ;///< 20 ноября. Предпразднство Введения (Входа) во храм Пресвятой Богородицы. Прп. Григория Декаполи́та. Свт. Про́кла, архиеп. Константинопольского.
oxc_const m11d21 = 1059  ;///< 21 ноября. Введе́ние (Вход) во храм Пресвятой Владычицы нашей Богородицы и Приснодевы Марии.
oxc_const m11d22 = 1060  ;///< 22 ноября. Попразднство Введения. Апп. от 70-ти Филимо́на, Архи́ппа и мц. равноап. Апфи́и.
oxc_const m11d23 = 1061  ;///< 23 ноября. Попразднство Введения. Блгв. вел. кн. Алекса́ндра Не́вского. Свт. Митрофа́на, в схиме Мака́рия, еп. Воро́нежского.
oxc_const m11d24 = 1062  ;///< 24 ноября. Попразднство Введения. Вмц. Екатерины. Вмч. Мерку́рия.
oxc_const m11d25 = 1063  ;///< 25 ноября. Отдание праздника Введения (Входа) во храм Пресвятой Богородицы. Сщмчч. Кли́мента, папы Римского, и Петра́, архиеп. Александри́йского.
oxc_const m12d20 = 1064  ;///< 20 декабря. Предпразднство Рождества Христова. Сщмч. Игна́тия Богоно́сца. Прав. Иоа́нна Кроншта́дтского.
oxc_const m12d21 = 1065  ;///< 21 декабря. Предпразднство Рождества Христова. Свт. Петра, митр. Киевского, Московского и всея Руси, чудотворца.
oxc_const m12d22 = 1066  ;///< 22 декабря. Предпразднство Рождества Христова. Вмц. Анастаси́и Узореши́тельницы.
oxc_const m12d23 = 1067  ;///< 23 декабря. Предпразднство Рождества Христова. Десяти мучеников, иже в Кри́те.
oxc_const m12d24 = 1068  ;///< 24 декабря. Предпразднство Рождества Христова. На́вечерие Рождества Христова (Рождественский сочельник). Прмц. Евге́нии.
oxc_const m12d25 = 1069  ;///< 25 декабря. Рождество Господа Бога и Спаса нашего Иисуса Христа.
oxc_const m12d26 = 1070  ;///< 26 декабря. Попразднство Рождества Христова. Собор Пресвятой Богородицы.
oxc_const m12d27 = 1071  ;///< 27 декабря. Попразднство Рождества Христова. Ап. первомч. и архидиа́кона Стефа́на. Прп. Фео́дора Начерта́нного, исп.
oxc_const m12d28 = 1072  ;///< 28 декабря. Попразднство Рождества Христова. Мучеников 20 000 в Никомидии сожженных
oxc_const m12d29 = 1073  ;///< 29 декабря. Попразднство Рождества Христова. Мучеников 14 000 младенцев, от Ирода в Вифлееме избиенных.
oxc_const m12d30 = 1074  ;///< 30 декабря. Попразднство Рождества Христова. Мц. Ани́сии. Прп. Мела́нии Ри́мляныни (с 31декабря). Свт. Мака́рия, митр. Московского.
oxc_const m12d31 = 1075  ;///< 31 декабря. Отдание праздника Рождества Христова.
/** @} */

/**
 * \defgroup block3 группа констант 3 - другие дни года
 * @{
 *
*/
oxc_const sub_peredbogoyav       = 2001  ;///< Суббота пред Богоявлением.
oxc_const ned_peredbogoyav       = 2003  ;///< Неделя пред Богоявлением.
oxc_const sub_pobogoyav          = 2004  ;///< Суббота по Богоявлении.
oxc_const ned_pobogoyav          = 2005  ;///< Неделя по Богоявлении.
oxc_const sobor_novom_rus        = 2006  ;///< Собор новомучеников и исповедников Церкви Русской.
oxc_const sobor_3sv              = 2007  ;///< 30января. Собор вселенских учителей и святителей Василия Великого, Григория Богослова и Иоанна Златоустого.
oxc_const sretenie_predpr        = 2008  ;///< 1 февраля. Предпразднство Сре́тения Господня
oxc_const sretenie               = 2009  ;///< 2 февраля. Сре́тение Господа Бога и Спаса нашего Иисуса Христа.
oxc_const sretenie_poprazd1      = 2010  ;///< Первый день попразднства Сре́тения Господня.
oxc_const sretenie_poprazd2      = 2011  ;///< Второй день попразднства Сре́тения Господня.
oxc_const sretenie_poprazd3      = 2012  ;///< Третий день попразднства Сре́тения Господня.
oxc_const sretenie_poprazd4      = 2013  ;///< Четвертый день попразднства Сре́тения Господня.
oxc_const sretenie_poprazd5      = 2014  ;///< Пятый день попразднства Сре́тения Господня.
oxc_const sretenie_poprazd6      = 2015  ;///< Шестой день попразднства Сре́тения Господня.
oxc_const sretenie_otdanie       = 2016  ;///< Oтдание праздника Сре́тения Господня. Eсли сретение в неделю блуднаго или в пн. или вт. мясопустныя недели - отдание в пт. тойже недели. Если сретение в ср. чт. пт. или сб. мясопустныя недели - отдание во вт. сырны. Если сретение в неделю мясопустную или в пн. сырныя - отдание в чт. тойже недели. Если сретение в вт. или ср. сырныя - отдание в сб. тойже недели. Если сретение в чт. пт. или сб. сырную - отдание в неделю сыропустную. Если сретение в неделю сыропустную тогда празднуем един день (типикон стр. 501). в других случаях отдание сретение - 9 февраля.
oxc_const obret_gl_ioanna12      = 2017  ;///< 24февраля. Первое и второе Обре́тение главы Иоанна Предтечи.
oxc_const muchenik_40            = 2018  ;///< 9 марта. Святых сорока́ мучеников, в Севасти́йском е́зере мучившихся. (типикон стр. 546)
oxc_const blag_predprazd         = 2019  ;///< 24 марта. предпразднество Благовещ́ение Пресвято́й Богоро́дицы.
oxc_const blag_otdanie           = 2020  ;///< 26 марта. отдание праздника Благовещ́ение Пресвято́й Богоро́дицы.
oxc_const georgia_pob            = 2021  ;///< 23 апреля. Вмч. Гео́ргия Победоно́сца. Мц. царицы Александры.
oxc_const obret_gl_ioanna3       = 2022  ;///< 25мая. третье Обре́тение главы Иоанна Предтечи.
oxc_const sobor_tversk           = 2023  ;///< Собор Тверских святых
oxc_const sobor_otcev_1_6sob     = 2024  ;///< Святых отец 6-и вселенских соборов. 16июля - если воскресение. Если пн, вт или ср - переносится на предыдущую неделю. Если чт пт или сб - на следующую
oxc_const sobor_kemero           = 2025  ;///< Собор Кемеровских святых
oxc_const pahomii_kensk          = 2026  ;///< Прп. Пахомия Кенского (XVI) (переходящее празднование в субботу по Богоявлении).
oxc_const shio_mg                = 2027  ;///< Прп.Шио Мгвимского (VI) (Груз.) (переходящее празднование в четверг сырнойседмицы).
oxc_const feodor_tir             = 2028  ;///< Вмч. Феодора Тирона (ок. 306) (переходящее празднование в субботу 1-й седмицы Великого поста).
oxc_const grigor_palam           = 2029  ;///< Свт. Григория Паламы, архиеп. Фессалонитского (переходящее празднование во 2-ю Неделю Великого поста).
oxc_const ioann_lestv            = 2030  ;///< Прп. Иоанна Лествичника (переходящее празднование в 4-ю Неделю Великого поста).
oxc_const mari_egipt             = 2031  ;///< Прп. Марии Египетской (переходящее празднование в 5-ю Неделю Великого поста).
oxc_const prep_dav_gar           = 2032  ;///< Преподобномучеников отцов Давидо-Гареджийских (1616) (Груз.)(переходящее празднование во вторник Светлой седмицы).
oxc_const hristodul              = 2033  ;///< Мчч. Христодула и Анастасии Патрских, убиенных в Ахаии (1821) (переходящее празднование вовторник Светлой седмицы).
oxc_const iosif_arimaf           = 2034  ;///< праведных Иосифа Аримафейского и Никодима(переходящее празднование в Неделю 3-ю по Пасхе).
oxc_const tamar_gruz             = 2035  ;///< Блгв. Тамары, царицы Грузинской (переходящее празднование в Неделюмироносиц).
oxc_const pm_avraam_bolg         = 2036  ;///< Перенесение мощей мч. Авраамия Бо'лгарского (1230)(переходящее празднование в Неделю 4-ю по Пасхе).
oxc_const tavif                  = 2037  ;///< Прав. Тавифы (I)(переходящее празднование в Неделю 4-ю по Пасхе).
oxc_const much_fereidan          = 2038  ;///< Мучеников, в долине Ферейдан (Иран) от персов пострадавших (XVII) (Груз.) (переходящее празднование в день ВознесенияГосподня).
oxc_const dodo_gar               = 2039  ;///< Прп. Додо Гареджийского (Груз.)(623) (переходящее празднование в среду по Вознесении).
oxc_const david_gar              = 2040  ;///< Прп. Давида Гареджийского (Груз.)(VI) (переходящеепразднование в четверг по Вознесении).
oxc_const prep_otec_afon         = 2041  ;///< Всех преподобных и богоносных отцов, во СвятойГоре Афонской просиявших (переходящее празднование в Неделю 2-ю поПятидесятнице).
oxc_const prep_sokolovsk         = 2042  ;///< Прпп. Тихона, Василия и Никона Соколовских(XVI) (переходящее празднование в 1-е воскресенье после 29 июня).
oxc_const arsen_tversk           = 2043  ;///< Свт.Арсения, еп. Тверского (переходящее празднование в 1-е воскресенье после 29июня).
oxc_const much_lipsiisk          = 2044  ;///< Прмчч. Неофита, Ионы, Неофита, Ионы и Парфения Липсийских(переходящее празднование в 1-е воскресенье после 27 июня).
oxc_const sub_porojdestve_r      = 2045  ;///< Чтения субботы по Рождестве Христовом.
oxc_const ned_porojdestve_r      = 2046  ;///< Чтения недели по Рождестве Христовом.
oxc_const sub_peredbogoyav_r     = 2047  ;///< Чтения субботы пред Богоявлением.
oxc_const ned_peredbogoyav_r     = 2048  ;///< Чтения недели пред Богоявлением.
/** @} */

/**
 * \defgroup block4 группа констант 4 - признаки принадлежности даты к множеству (по праздникам)
 * @{
 *
*/
oxc_const dvana10_per_prazd     = 3001; ///< Двунадесятые переходящие праздники
oxc_const dvana10_nep_prazd     = 3002; ///< Двунадесятые непереходящие праздники
oxc_const vel_prazd             = 3003; ///< Великие праздники
/** @} */

/**
 * \defgroup block5 группа констант 5 - признаки принадлежности даты к множеству (по многодневным постам и сплошным седмицам)
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

}// namespace oxc

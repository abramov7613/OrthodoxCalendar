#include "oxc.h"
#include <iostream>
#include <map>
#include <set>
#include <array>
#include <queue>
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace oxc {

using ShortDate = std::pair<int8_t, int8_t> ; //first = month, second = day
using ApEvReads = OrthodoxCalendar::ApostolEvangelieReadings ;
ShortDate pasha_calc(int year)
{
	auto [y, m, d] = OrthodoxCalendar::pascha(year);
	return {m, d};
}

/*----------------------------------------------*/
/*              class OrthYear                  */
/*----------------------------------------------*/

class OrthYear {

	struct Data1 {
		int8_t dn{-1};
		int8_t glas{-1};
		int8_t n50{-1};
		int8_t day{};
		int8_t month{};
		ApEvReads apostol;
		ApEvReads evangelie;
		std::array<uint16_t, 8> day_markers{};
		bool operator<(const Data1& rhs) const
		{
			return ShortDate{month, day} < ShortDate{rhs.month, rhs.day};
		}
		bool operator==(const Data1& rhs) const = default;
		bool operator<(const ShortDate& rhs) const
		{
			return ShortDate{month, day} < rhs;
		}
		bool operator==(const ShortDate& rhs) const
		{
			return ShortDate{month, day} == rhs;
		}
	};
	struct Data2 {
		uint16_t marker{};
		int8_t day{};
		int8_t month{};
		bool operator<(const Data2& rhs) const
		{
			return marker < rhs.marker;
		}
		bool operator==(const Data2& rhs) const = default;
		bool operator<(const uint16_t rhs) const
		{
			return marker < rhs;
		}
		bool operator==(const uint16_t rhs) const
		{
			return marker == rhs;
		}
		friend bool operator<(const uint16_t lhs, const Data2& rhs)
		{
			return lhs < rhs.marker;
		}
	};
	std::vector<Data1> data1;
	std::vector<Data2> data2;
	int8_t winter_indent;
	int8_t spring_indent;
	std::optional<decltype(data1)::const_iterator> find_in_data1(int8_t m, int8_t d) const;

public:
	/**
	 * 	\brief Основной Конструктор объекта
	 * 	\param [in] y число года в диапазоне от 2 до INT_MAX-1
	 * 	\param [in] il список номеров добавочных седмиц при вычислении зимней / осенней отступкu литургийных чтений.
	 *  \param [in] osen_otstupka_apostol признак указывающий учитывать ли апостол при вычислении осенней отступкu
	 *  \throw std::runtime_error в случае неверного диапазона входных параметров или если il.size()!=17 или если (il[i] < 1 || il[i] > 33)
	 *
	 *	Параметр il должен содержать 17 элементов:<br>
	 *   первый элемент (index = 0) - номер добавочной седмицы зимней отступкu при отступке в 1 седмиц.<br>
	 *   второй и третий - номера добавочных седмиц зимней отступкu при отступке в 2 седмиц.<br>
	 *   следующие 3 элемента - номера добавочных седмиц зимней отступкu при отступке в 3 седмиц.<br>
	 *   следующие 4 элемента - номера добавочных седмиц зимней отступкu при отступке в 4 седмиц.<br>
	 *   следующие 5 элемента - номера добавочных седмиц зимней отступкu при отступке в 5 седмиц.<br>
	 *   последние 2 элемента - номера добавочных седмиц осенней отступкu.<br>
	 */
	OrthYear(int y, std::span<const int> il, bool osen_otstupka_apostol);
	///Конструктор с параметрами по умолчанию
	OrthYear(int y, bool osen_otstupka_apostol)
		: OrthYear(y, std::array{33,32,33,31,32,33,30,31,32,33,30,31,17,32,33,10,11}, osen_otstupka_apostol) {}
	///Конструктор с параметрами по умолчанию
	OrthYear(int y): OrthYear(y, false) {}
	///Конструктор с параметрами по умолчанию
	OrthYear(int y, std::span<const int> il): OrthYear(y, il, false) {}

	int8_t get_winter_indent() const;
	int8_t get_spring_indent() const;
	int8_t get_date_glas(int8_t month, int8_t day) const;
	int8_t get_date_n50(int8_t month, int8_t day) const;
	int8_t get_date_dn(int8_t month, int8_t day) const;
	ApEvReads get_date_apostol(int8_t month, int8_t day) const;
	ApEvReads get_date_evangelie(int8_t month, int8_t day) const;
	ApEvReads get_resurrect_evangelie(int8_t month, int8_t day) const;
	std::optional<std::vector<uint16_t>> get_date_properties(int8_t month, int8_t day) const;
	std::optional<ShortDate> get_date_with(uint16_t m) const;
	std::optional<std::vector<ShortDate>> get_alldates_with(uint16_t m) const;
	std::optional<ShortDate> get_date_withanyof(std::span<uint16_t> m) const;
	std::optional<ShortDate> get_date_withallof(std::span<uint16_t> m) const;
	std::optional<std::vector<ShortDate>> get_alldates_withanyof(std::span<uint16_t> m) const;
	std::string get_description_forday(int8_t month, int8_t day) const;
};

OrthYear::OrthYear(int y, std::span<const int> il, bool osen_otstupka_apostol)
{//main constructor
	if( (y < 2) || (y > std::numeric_limits<int>::max() - 1) )
	//"выход числа года за границу диапазона"
		throw std::runtime_error(std::to_string(y)+' '+std::to_string(std::numeric_limits<int>::max() - 1));
	bool bad_il{};
	for(auto j: il) if(j<1 || j>33) bad_il = true;
	if(il.size()!=17 || bad_il)
		throw std::runtime_error("установлены некорректные параметры отступки/преступки апостольских/евангельских чтений");
	//таблица - непереходящие даты года
	static const std::array<int, 225> stable_dates  = {
		m1d1, 1, 1,
		m1d2, 1, 2,
		m1d3, 1, 3,
		m1d4, 1, 4,
		m1d5, 1, 5,
		m1d6, 1, 6,
		m1d7, 1, 7,
		m1d8, 1, 8,
		m1d9, 1, 9,
		m1d10, 1, 10,
		m1d11, 1, 11,
		m1d12, 1, 12,
		m1d13, 1, 13,
		m1d14, 1, 14,
		m3d25, 3, 25,
		m5d11, 5, 11,
		m6d24, 6, 24,
		m6d25, 6, 25,
		m6d29, 6, 29,
		m6d30, 6, 30,
		m7d15, 7, 15,
		m8d5, 8, 5,
		m8d6, 8, 6,
		m8d7, 8, 7,
		m8d8, 8, 8,
		m8d9, 8, 9,
		m8d10, 8, 10,
		m8d11, 8, 11,
		m8d12, 8, 12,
		m8d13, 8, 13,
		m8d14, 8, 14,
		m8d15, 8, 15,
		m8d16, 8, 16,
		m8d17, 8, 17,
		m8d18, 8, 18,
		m8d19, 8, 19,
		m8d20, 8, 20,
		m8d21, 8, 21,
		m8d22, 8, 22,
		m8d23, 8, 23,
		m9d7, 9, 7,
		m9d8, 9, 8,
		m9d9, 9, 9,
		m9d10, 9, 10,
		m9d11, 9, 11,
		m9d12, 9, 12,
		m9d13, 9, 13,
		m9d14, 9, 14,
		m9d15, 9, 15,
		m9d16, 9, 16,
		m9d17, 9, 17,
		m9d18, 9, 18,
		m9d19, 9, 19,
		m9d20, 9, 20,
		m9d21, 9, 21,
		m8d29, 8, 29,
		m10d1, 10, 1,
		m11d20, 11, 20,
		m11d21, 11, 21,
		m11d22, 11, 22,
		m11d23, 11, 23,
		m11d24, 11, 24,
		m11d25, 11, 25,
		m12d20, 12, 20,
		m12d21, 12, 21,
		m12d22, 12, 22,
		m12d23, 12, 23,
		m12d24, 12, 24,
		m12d25, 12, 25,
		m12d26, 12, 26,
		m12d27, 12, 27,
		m12d28, 12, 28,
		m12d29, 12, 29,
		m12d30, 12, 30,
		m12d31, 12, 31
	};
	auto make_pair = [](int m, int d){ return ShortDate{m,d}; };
	//таблица - даты сплошных седмиц
	static const std::array svyatki_dates = {
		make_pair(1,1),
		make_pair(1,2),
		make_pair(1,3),
		make_pair(1,4),
		make_pair(12,25),
		make_pair(12,26),
		make_pair(12,27),
		make_pair(12,28),
		make_pair(12,29),
		make_pair(12,30),
		make_pair(12,31)
	};
	//type alias for const tables
	using TT1 = std::array<std::array<ApEvReads, 7>, 37>;
	using TT2 = std::map<uint16_t, ApEvReads>;
	//таблица рядовых чтений на литургии из приложения богосл.евангелия. период от св. троицы до нед. сыропустная
	//двумерный массив [a][b], где а - календарный номер по пятидесятнице. b - деньнедели.
	static const TT1 evangelie_table_1 {
		std::array { ApEvReads{27,"Ин., 27 зач., VII, 37–52; VIII, 12."},	//неделя 0. день св. троицы
						ApEvReads{},
						ApEvReads{},
						ApEvReads{},
						ApEvReads{},
						ApEvReads{},
						ApEvReads{}
					},
		std::array { ApEvReads{38,"Мф., 38 зач., X, 32–33, 37–38; XIX, 27–30."},	//Неделя 1 всех святых
						ApEvReads{75,"Мф., 75 зач., XVIII, 10–20."},	//пн - Святаго Духа
						ApEvReads{10,"Мф., 10 зач., IV, 25 – V, 12."},	//вт - седмица 1
						ApEvReads{12,"Мф., 12 зач., V, 20–26."},	//ср
						ApEvReads{13,"Мф., 13 зач., V, 27–32."},	//чт
						ApEvReads{14,"Мф., 14 зач., V, 33–41."},	//пт
						ApEvReads{15,"Мф., 15 зач., V, 42–48."}	//сб
					},
		std::array { ApEvReads{ 9,"Мф., 9 зач., IV, 18–23."},	//Неделя 2
						ApEvReads{19,"Мф., 19 зач., VI, 31–34; VII, 9–11."},	//пн - седмица 2
						ApEvReads{22,"Мф., 22 зач., VII, 15–21."},	//вт
						ApEvReads{23,"Мф., 23 зач., VII, 21–23."},	//ср
						ApEvReads{27,"Мф., 27 зач., VIII, 23–27."},	//чт
						ApEvReads{31,"Мф., 31 зач., IX, 14–17."},	//пт
						ApEvReads{20,"Мф., 20 зач., VII, 1–8."}	//сб
					},
		std::array { ApEvReads{18,"Мф., 18 зач., VI, 22–33."},	//Неделя 3
						ApEvReads{34,"Мф., 34 зач., IX, 36 – X, 8."},	//пн - седмица 3
						ApEvReads{35,"Мф., 35 зач., X, 9–15."},	//вт
						ApEvReads{36,"Мф., 36 зач., X, 16–22."},	//ср
						ApEvReads{37,"Мф., 37 зач., X, 23–31."},	//чт
						ApEvReads{38,"Мф., 38 зач., X, 32–36; XI, 1."},	//пт
						ApEvReads{24,"Мф., 24 зач., VII, 24 – VIII, 4."}	//сб
					},
		std::array { ApEvReads{25,"Мф., 25 зач., VIII, 5–13."},	//Неделя 4
						ApEvReads{40,"Мф., 40 зач., XI, 2–15."},	//пн - седмица 4
						ApEvReads{41,"Мф., 41 зач., XI, 16–20."},	//вт
						ApEvReads{42,"Мф., 42 зач., XI, 20–26."},	//ср
						ApEvReads{43,"Мф., 43 зач., XI, 27–30."},	//чт
						ApEvReads{44,"Мф., 44 зач., XII, 1–8."},	//пт
						ApEvReads{26,"Мф., 26 зач., VIII, 14–23."}	//сб
					},
		std::array { ApEvReads{28,"Мф., 28 зач., VIII, 28 - IX, 1."},	//Неделя 5
						ApEvReads{45,"Мф., 45 зач., XII, 9-13."},	//пн - седмица 5
						ApEvReads{46,"Мф., 46 зач., XII, 14–16, 22–30."},	//вт
						ApEvReads{48,"Мф., 48 зач., XII, 38–45."},	//ср
						ApEvReads{49,"Мф., 49 зач., XII, 46 – XIII, 3."},	//чт
						ApEvReads{50,"Мф., 50 зач., XIII, 3–9."},	//пт
						ApEvReads{30,"Мф., 30 зач., IX, 9–13."}	//сб
					},
		std::array { ApEvReads{29,"Мф., 29 зач., IX, 1–8."},	//Неделя 6
						ApEvReads{51,"Мф., 51 зач., XIII, 10–23."},	//пн - седмица 6
						ApEvReads{52,"Мф., 52 зач., XIII, 24–30."},	//вт
						ApEvReads{53,"Мф., 53 зач., XIII, 31–36."},	//ср
						ApEvReads{54,"Мф., 54 зач., XIII, 36–43."},	//чт
						ApEvReads{55,"Мф., 55 зач., XIII, 44–54."},	//пт
						ApEvReads{32,"Мф., 32 зач., IX, 18–26."}	//сб
					},
		std::array { ApEvReads{33,"Мф., 33 зач., IX, 27–35."},	//Неделя 7
						ApEvReads{56,"Мф., 56 зач., XIII, 54–58."},	//пн - седмица 7
						ApEvReads{57,"Мф., 57 зач., XIV, 1–13."},	//вт
						ApEvReads{60,"Мф., 60 зач., XIV, 35 – XV, 11."},	//ср
						ApEvReads{61,"Мф., 61 зач., XV, 12–21."},	//чт
						ApEvReads{63,"Мф., 63 зач., XV, 29–31."},	//пт
						ApEvReads{39,"Мф., 39 зач., X, 37 – XI, 1."}	//сб
					},
		std::array { ApEvReads{58,"Мф., 58 зач., XIV, 14–22."},	//Неделя 8
						ApEvReads{65,"Мф., 65 зач., XVI, 1-6."},	//пн - седмица 8
						ApEvReads{66,"Мф., 66 зач., XVI, 6-12."},	//вт
						ApEvReads{68,"Мф., 68 зач., XVI, 20–24."},	//ср
						ApEvReads{69,"Мф., 69 зач., XVI, 24–28."},	//чт
						ApEvReads{71,"Мф., 71 зач., XVII, 10-18."},	//пт
						ApEvReads{47,"Мф., 47 зач., XII, 30–37."}	//сб
					},
		std::array { ApEvReads{59,"Мф., 59 зач., XIV, 22–34."},	//Неделя 9
						ApEvReads{74,"Мф., 74 зач., XVIII, 1–11."},	//пн - седмица 9
						ApEvReads{76,"Мф., 76 зач., XVIII, 18–22; XIX, 1–2, 13–15."},	//вт
						ApEvReads{80,"Мф., 80 зач., XX, 1–16."},	//ср
						ApEvReads{81,"Мф., 81 зач., XX, 17–28."},	//чт
						ApEvReads{83,"Мф., 83 зач., XXI, 1–11, 15–17."},	//пт
						ApEvReads{64,"Мф., 64 зач., XV, 32–39."}	//сб
					},
		std::array { ApEvReads{72,"Мф., 72 зач., XVII, 14–23."},	//Неделя 10
						ApEvReads{84,"Мф., 84 зач., XXI, 18–22."},	//пн - седмица 10
						ApEvReads{85,"Мф., 85 зач., XXI, 23–27."},	//вт
						ApEvReads{86,"Мф., 86 зач., XXI, 28–32."},	//ср
						ApEvReads{88,"Мф., 88 зач., XXI, 43-46."},	//чт
						ApEvReads{91,"Мф., 91 зач., XXII, 23–33."},	//пт
						ApEvReads{73,"Мф., 73 зач., XVII, 24 – XVIII, 4."}	//сб
					},
		std::array { ApEvReads{77,"Мф., 77 зач., XVIII, 23–35."},	//Неделя 11
						ApEvReads{94,"Мф., 94 зач., XXIII, 13–22."},	//пн - седмица 11
						ApEvReads{95,"Мф., 95 зач., XXIII, 23-28."},	//вт
						ApEvReads{96,"Мф., 96 зач., XXIII, 29–39."},	//ср
						ApEvReads{99,"Мф., 99 зач., XXIV, 13–28."},	//чт
						ApEvReads{100,"Мф., 100 зач., XXIV, 27–33, 42–51."},	//пт
						ApEvReads{78,"Мф., 78 зач., XIX, 3–12."}	//сб
					},
		std::array { ApEvReads{79,"Мф., 79 зач., XIX, 16–26."},	//Неделя 12
						ApEvReads{2,"Мк., 2 зач., I, 9–15."},	//пн - седмица 12
						ApEvReads{3,"Мк., 3 зач., I, 16–22."},	//вт
						ApEvReads{4,"Мк., 4 зач., I, 23–28."},	//ср
						ApEvReads{5,"Мк., 5 зач., I, 29-35."},	//чт
						ApEvReads{9,"Мк., 9 зач., II, 18–22."},	//пт
						ApEvReads{82,"Мф., 82 зач., XX, 29–34."}	//сб
					},
		std::array { ApEvReads{87,"Мф., 87 зач., XXI, 33–42."},	//Неделя 13
						ApEvReads{11,"Мк., 11 зач., III, 6–12."},	//пн - седмица 13
						ApEvReads{12,"Мк., 12 зач., III, 13–19."},	//вт
						ApEvReads{13,"Мк., 13 зач., III, 20–27."},	//ср
						ApEvReads{14,"Мк., 14 зач., III, 28–35."},	//чт
						ApEvReads{15,"Мк., 15 зач., IV, 1–9."},	//пт
						ApEvReads{90,"Мф., 90 зач., XXII, 15-22."}	//сб
					},
		std::array { ApEvReads{89,"Мф., 89 зач., XXII, 1–14."},	//Неделя 14
						ApEvReads{16,"Мк., 16 зач., IV, 10–23."},	//пн - седмица 14
						ApEvReads{17,"Мк., 17 зач., IV, 24–34."},	//вт
						ApEvReads{18,"Мк., 18 зач., IV, 35–41."},	//ср
						ApEvReads{19,"Мк., 19 зач., V, 1-20."},	//чт
						ApEvReads{20,"Мк., 20 зач., V, 22–24, 35 – VI, 1."},	//пт
						ApEvReads{93,"Мф., 93 зач., XXIII, 1–12."}	//сб
					},
		std::array { ApEvReads{92,"Мф., 92 зач., XXII, 35–46."},	//Неделя 15
						ApEvReads{21,"Мк., 21 зач., V, 24–34."},	//пн - седмица 15
						ApEvReads{22,"Мк., 22 зач., VI, 1-7."},	//вт
						ApEvReads{23,"Мк., 23 зач., VI, 7–13."},	//ср
						ApEvReads{25,"Мк., 25 зач., VI, 30–45."},	//чт
						ApEvReads{26,"Мк., 26 зач., VI, 45–53."},	//пт
						ApEvReads{97,"Мф., 97 зач., XXIV, 1–13."}	//сб
					},
		std::array { ApEvReads{105,"Мф., 105 зач., XXV, 14-30."},	//Неделя 16
						ApEvReads{27,"Мк., 27 зач., VI, 54 - VII, 8."},	//пн - седмица 16
						ApEvReads{28,"Мк., 28 зач., VII, 5-16."},	//вт
						ApEvReads{29,"Мк., 29 зач., VII, 14–24."},	//ср
						ApEvReads{30,"Мк., 30 зач., VII, 24–30."},	//чт
						ApEvReads{32,"Мк., 32 зач., VIII, 1-10."},	//пт
						ApEvReads{101,"Мф., 101 зач., XXIV, 34–44."}	//сб
					},
		std::array { ApEvReads{62,"Мф., 62 зач., XV, 21–28."},	//Неделя 17
						ApEvReads{48,"Мк., 48 зач., X, 46–52."},	//пн - седмица 17
						ApEvReads{50,"Мк., 50 зач., XI, 11–23."},	//вт
						ApEvReads{51,"Мк., 51 зач., XI, 23–26."},	//ср
						ApEvReads{52,"Мк., 52 зач., XI, 27–33."},	//чт
						ApEvReads{53,"Мк., 53 зач., XII, 1–12."},	//пт
						ApEvReads{104,"Мф., 104 зач., XXV, 1–13."}	//сб
					},
		std::array { ApEvReads{17,"Лк., 17 зач., V, 1–11."},	//Неделя 18
						ApEvReads{10,"Лк., 10 зач., III, 19–22."},	//пн - седмица 18
						ApEvReads{11,"Лк., 11 зач., III, 23 – IV, 1."},	//вт
						ApEvReads{12,"Лк., 12 зач., IV, 1-15."},	//ср
						ApEvReads{13,"Лк., 13 зач., IV, 16–22."},	//чт
						ApEvReads{14,"Лк., 14 зач., IV, 22–30."},	//пт
						ApEvReads{15,"Лк., 15 зач., IV, 31–36."}	//сб
					},
		std::array { ApEvReads{26,"Лк., 26 зач., VI, 31–36."},	//Неделя 19
						ApEvReads{16,"Лк., 16 зач., IV, 37–44."},	//пн - седмица 19
						ApEvReads{18,"Лк., 18 зач., V, 12-16."},	//вт
						ApEvReads{21,"Лк., 21 зач., V, 33–39."},	//ср
						ApEvReads{23,"Лк., 23 зач., VI, 12–19."},	//чт
						ApEvReads{24,"Лк., 24 зач., VI, 17–23."},	//пт
						ApEvReads{19,"Лк., 19 зач., V, 17–26."}	//сб
					},
		std::array { ApEvReads{30,"Лк., 30 зач., VII, 11–16."},	//Неделя 20
						ApEvReads{25,"Лк., 25 зач., VI, 24–30."},	//пн - седмица 20
						ApEvReads{27,"Лк., 27 зач., VI, 37–45."},	//вт
						ApEvReads{28,"Лк., 28 зач., VI, 46 – VII, 1."},	//ср
						ApEvReads{31,"Лк., 31 зач., VII, 17–30."},	//чт
						ApEvReads{32,"Лк., 32 зач., VII, 31–35."},	//пт
						ApEvReads{20,"Лк., 20 зач., V, 27–32."}	//сб
					},
		std::array { ApEvReads{35,"Лк., 35 зач., VIII, 5–15."},	//Неделя 21
						ApEvReads{33,"Лк., 33 зач., VII, 36–50."},	//пн - седмица 21
						ApEvReads{34,"Лк., 34 зач., VIII, 1–3."},	//вт
						ApEvReads{37,"Лк., 37 зач., VIII, 22–25."},	//ср
						ApEvReads{41,"Лк., 41 зач., IX, 7–11."},	//чт
						ApEvReads{42,"Лк., 42 зач., IX, 12–18."},	//пт
						ApEvReads{22,"Лк., 22 зач., VI, 1–10."}	//сб
					},
		std::array { ApEvReads{83,"Лк., 83 зач., XVI, 19–31."},	//Неделя 22
						ApEvReads{43,"Лк., 43 зач., IX, 18–22."},	//пн - седмица 22
						ApEvReads{44,"Лк., 44 зач., IX, 23-27."},	//вт
						ApEvReads{47,"Лк., 47 зач., IX, 44–50."},	//ср
						ApEvReads{48,"Лк., 48 зач., IX, 49–56."},	//чт
						ApEvReads{50,"Лк., 50 зач., X, 1–15."},	//пт
						ApEvReads{29,"Лк., 29 зач., VII, 1–10."}	//сб
					},
		std::array { ApEvReads{38,"Лк., 38 зач., VIII, 26–39."},	//Неделя 23
						ApEvReads{52,"Лк., 52 зач., X, 22–24."},	//пн - седмица 23
						ApEvReads{55,"Лк., 55 зач., XI, 1–10."},	//вт
						ApEvReads{56,"Лк., 56 зач., XI, 9–13."},	//ср
						ApEvReads{57,"Лк., 57 зач., XI, 14–23."},	//чт
						ApEvReads{58,"Лк., 58 зач., XI, 23–26."},	//пт
						ApEvReads{36,"Лк., 36 зач., VIII, 16–21."}	//сб
					},
		std::array { ApEvReads{39,"Лк., 39 зач., VIII, 41–56."},	//Неделя 24
						ApEvReads{59,"Лк., 59 зач., XI, 29–33."},	//пн - седмица 24
						ApEvReads{60,"Лк., 60 зач., XI, 34–41."},	//вт
						ApEvReads{61,"Лк., 61 зач., XI, 42–46."},	//ср
						ApEvReads{62,"Лк., 62 зач., XI, 47 – XII, 1."},	//чт
						ApEvReads{63,"Лк., 63 зач., XII, 2–12."},	//пт
						ApEvReads{40,"Лк., 40 зач., IX, 1–6."}	//сб
					},
		std::array { ApEvReads{53,"Лк., 53 зач., X, 25–37."},	//Неделя 25
						ApEvReads{65,"Лк., 65 зач., XII, 13–15, 22–31."},	//пн - седмица 25
						ApEvReads{68,"Лк., 68 зач., XII, 42–48."},	//вт
						ApEvReads{69,"Лк., 69 зач., XII, 48-59."},	//ср
						ApEvReads{70,"Лк., 70 зач., XIII, 1–9."},	//чт
						ApEvReads{73,"Лк., 73 зач., XIII, 31–35."},	//пт
						ApEvReads{46,"Лк., 46 зач., IX, 37–43."}	//сб
					},
		std::array { ApEvReads{66,"Лк., 66 зач., XII, 16–21."},	//Неделя 26
						ApEvReads{75,"Лк., 75 зач., XIV, 12–15."},	//пн - седмица 26
						ApEvReads{77,"Лк., 77 зач., XIV, 25–35."},	//вт
						ApEvReads{78,"Лк., 78 зач., XV, 1–10."},	//ср
						ApEvReads{80,"Лк., 80 зач., XVI, 1-9."},	//чт
						ApEvReads{82,"Лк., 82 зач., XVI, 15–18; XVII, 1–4."},	//пт
						ApEvReads{49,"Лк., 49 зач., IX, 57–62."}	//сб
					},
		std::array { ApEvReads{71,"Лк., 71 зач., XIII, 10–17."},	//Неделя 27
						ApEvReads{86,"Лк., 86 зач., XVII, 20–25."},	//пн - седмица 27
						ApEvReads{87,"Лк., 87 зач., XVII, 26–37."},	//вт
						ApEvReads{90,"Лк., 90 зач., XVIII, 15–17, 26–30."},	//ср
						ApEvReads{92,"Лк., 92 зач., XVIII, 31–34."},	//чт
						ApEvReads{95,"Лк., 95 зач., XIX, 12–28."},	//пт
						ApEvReads{51,"Лк., 51 зач., X, 16–21."}	//сб
					},
		std::array { ApEvReads{76,"Лк., 76 зач., XIV, 16–24."},	//Неделя 28
						ApEvReads{97,"Лк., 97 зач., XIX, 37–44."},	//пн - седмица 28
						ApEvReads{98,"Лк., 98 зач., XIX, 45–48."},	//вт
						ApEvReads{99,"Лк., 99 зач., XX, 1–8."},	//ср
						ApEvReads{100,"Лк., 100 зач., XX, 9–18."},	//чт
						ApEvReads{101,"Лк., 101 зач., XX, 19-26."},	//пт
						ApEvReads{67,"Лк., 67 зач., XII, 32–40."}	//сб
					},
		std::array { ApEvReads{85,"Лк., 85 зач., XVII, 12–19."},	//Неделя 29
						ApEvReads{102,"Лк., 102 зач., XX, 27–44."},	//пн - седмица 29
						ApEvReads{106,"Лк., 106 зач., XXI, 12–19."},	//вт
						ApEvReads{104,"Лк., 104 зач., XXI, 5–7, 10–11, 20–24."},	//ср
						ApEvReads{107,"Лк., 107 зач., XXI, 28–33."},	//чт
						ApEvReads{108,"Лк., 108 зач., XXI, 37 – XXII, 8."},	//пт
						ApEvReads{72,"Лк., 72 зач., XIII, 18–29."}	//сб
					},
		std::array { ApEvReads{91,"Лк., 91 зач., XVIII, 18-27."},	//Неделя 30
						ApEvReads{33,"Мк., 33 зач., VIII, 11–21."},	//пн - седмица 30
						ApEvReads{34,"Мк., 34 зач., VIII, 22–26."},	//вт
						ApEvReads{36,"Мк., 36 зач., VIII, 30–34."},	//ср
						ApEvReads{39,"Мк., 39 зач., IX, 10–16."},	//чт
						ApEvReads{41,"Мк., 41 зач., IX, 33–41."},	//пт
						ApEvReads{74,"Лк., 74 зач., XIV, 1–11."}	//сб
					},
		std::array { ApEvReads{93,"Лк., 93 зач., XVIII, 35-43."},	//Неделя 31
						ApEvReads{42,"Мк., 42 зач., IX, 42 – X, 1."},	//пн - седмица 31
						ApEvReads{43,"Мк., 43 зач., X, 2–12."},	//вт
						ApEvReads{44,"Мк., 44 зач., X, 11–16."},	//ср
						ApEvReads{45,"Мк., 45 зач., X, 17–27."},	//чт
						ApEvReads{46,"Мк., 46 зач., X, 23–32."},	//пт
						ApEvReads{81,"Лк., 81 зач., XVI, 10–15."}	//сб
					},
		std::array { ApEvReads{94,"Лк., 94 зач., XIX, 1-10."},	//Неделя 32
						ApEvReads{48,"Мк., 48 зач., X, 46–52."},	//пн - седмица 32
						ApEvReads{50,"Мк., 50 зач., XI, 11–23."},	//вт
						ApEvReads{51,"Мк., 51 зач., XI, 23–26."},	//ср
						ApEvReads{52,"Мк., 52 зач., XI, 27–33."},	//чт
						ApEvReads{53,"Мк., 53 зач., XII, 1–12."},	//пт
						ApEvReads{84,"Лк., 84 зач., XVII, 3–10."}	//сб
					},
		std::array { ApEvReads{89,"Лк., 89 зач., XVIII, 10–14."},	//Неделя 33 о мытари и фарисеи
						ApEvReads{54,"Мк., 54 зач., XII, 13–17."},	//пн - седмица 33
						ApEvReads{55,"Мк., 55 зач., XII, 18–27."},	//вт
						ApEvReads{56,"Мк., 56 зач., XII, 28–37."},	//ср
						ApEvReads{57,"Мк., 57 зач., XII, 38–44."},	//чт
						ApEvReads{58,"Мк., 58 зач., XIII, 1–8."},	//пт
						ApEvReads{88,"Лк., 88 зач., XVIII, 2–8."}	//сб
					},
		std::array { ApEvReads{79,"Лк., 79 зач., XV, 11–32."},	//Неделя 34 о блуднем сыне
						ApEvReads{59,"Мк., 59 зач., XIII, 9–13."},	//пн - седмица 34
						ApEvReads{60,"Мк., 60 зач., XIII, 14-23."},	//вт
						ApEvReads{61,"Мк., 61 зач., XIII, 24–31."},	//ср
						ApEvReads{62,"Мк., 62 зач., XIII, 31 – XIV, 2."},	//чт
						ApEvReads{63,"Мк., 63 зач., XIV, 3-9."},	//пт
						ApEvReads{103,"Лк., 103 зач., XX, 45 – XXI, 4."}	//сб
					},
		std::array { ApEvReads{106,"Мф., 106 зач., XXV, 31–46."},	//Неделя 35 мясопустная
						ApEvReads{49,"Мк., 49 зач., XI, 1–11."},	//пн - седмица 35
						ApEvReads{64,"Мк., 64 зач., XIV, 10–42."},	//вт
						ApEvReads{65,"Мк., 65 зач., XIV, 43 – XV, 1."},	//ср
						ApEvReads{66,"Мк., 66 зач., XV, 1–15."},	//чт
						ApEvReads{68,"Мк., 68 зач., XV, 22, 25, 33–41."},	//пт
						ApEvReads{105,"Лк., 105 зач., XXI, 8–9, 25–27, 33–36."}	//сб
					},
		std::array { ApEvReads{ 17,"Мф., 17 зач., VI, 14–21."},	//Неделя 36 сыропустная
						ApEvReads{ 96,"Лк., 96 зач., XIX, 29–40; XXII, 7–39."},	//пн - седмица 36
						ApEvReads{109,"Лк., 109 зач., XXII, 39–42, 45 – XXIII, 1."},	//вт
						ApEvReads{},	//ср
						ApEvReads{110,"Лк., 110 зач., XXIII, 1–34, 44–56."},	//чт
						ApEvReads{},	//пт
						ApEvReads{ 16,"Мф., 16 зач., VI, 1–13."}	//сб
					}
	};
	auto evangelie_table1_get_chteniya = [](int8_t n50, int8_t dn)->ApEvReads {
		const ApEvReads & ref = evangelie_table_1.at(n50).at(dn);
		return {ref.n, ref.c};
	};
	//таблица рядовых чтений на литургии из приложения богосл.апостола. период от св. троицы до нед. сыропустная
	//двумерный массив [a][b], где а - календарный номер по пятидесятнице. b - деньнедели.
	static const TT1 apostol_table_1 {
		std::array { ApEvReads{ 3,"Деян., 3 зач., II, 1–11."},	//неделя 0. день св. троицы
						ApEvReads{},
						ApEvReads{},
						ApEvReads{},
						ApEvReads{},
						ApEvReads{},
						ApEvReads{}
					},
		std::array { ApEvReads{330,"Евр., 330 зач., XI, 33 – XII, 2."},	//Неделя 1 всех святых
						ApEvReads{229,"Еф., 229 зач., V, 8–19."},	//пн - Святаго Духа
						ApEvReads{79,"Рим., 79 зач., I, 1–7, 13–17."},	//вт - седмица 1
						ApEvReads{80,"Рим., 80 зач., I, 18–27."},	//ср
						ApEvReads{81,"Рим., 81 зач., I, 28 – II, 9."},	//чт
						ApEvReads{82,"Рим., 82 зач., II, 14–29."},	//пт
						ApEvReads{79,"Рим., 79 зач., I, 7-12."}	//сб
					},
		std::array { ApEvReads{81,"Рим., 81 зач., II, 10-16."},	//Неделя 2
						ApEvReads{83,"Рим., 83 зач., II, 28 – III, 18."},	//пн - седмица 2
						ApEvReads{86,"Рим., 86 зач., IV, 4–12."},	//вт
						ApEvReads{87,"Рим., 87 зач., IV, 13–25."},	//ср
						ApEvReads{89,"Рим., 89 зач., V, 10–16."},	//чт
						ApEvReads{90,"Рим., 90 зач., V, 17 – VI, 2."},	//пт
						ApEvReads{84,"Рим., 84 зач., III, 19–26."}	//сб
					},
		std::array { ApEvReads{88,"Рим., 88 зач., V, 1–10."},	//Неделя 3
						ApEvReads{94,"Рим., 94 зач., VII, 1–13."},	//пн - седмица 3
						ApEvReads{95,"Рим., 95 зач., VII, 14 – VIII, 2."},	//вт
						ApEvReads{96,"Рим., 96 зач., VIII, 2–13."},//ср
						ApEvReads{98,"Рим., 98 зач., VIII, 22–27."},	//чт
						ApEvReads{101,"Рим., 101 зач., IX, 6–19."},	//пт
						ApEvReads{85,"Рим., 85 зач., III, 28 – IV, 3."}	//сб
					},
		std::array { ApEvReads{93,"Рим., 93 зач., VI, 18-23."},	//Неделя 4
						ApEvReads{102,"Рим., 102 зач., IX, 18–33."},	//пн - седмица 4
						ApEvReads{104,"Рим., 104 зач., X, 11 – XI, 2."},	//вт
						ApEvReads{105,"Рим., 105 зач., XI, 2–12."},	//ср
						ApEvReads{106,"Рим., 106 зач., XI, 13–24."},	//чт
						ApEvReads{107,"Рим., 107 зач., XI, 25–36."},	//пт
						ApEvReads{ 92,"Рим., 92 зач., VI, 11–17."}	//сб
					},
		std::array { ApEvReads{103,"Рим., 103 зач., X, 1–10."},	//Неделя 5
						ApEvReads{109,"Рим., 109 зач., XII, 4–5, 15–21."},	//пн - седмица 5
						ApEvReads{114,"Рим., 114 зач., XIV, 9–18."},	//вт
						ApEvReads{117,"Рим., 117 зач., XV, 7–16."},	//ср
						ApEvReads{118,"Рим., 118 зач., XV, 17–29."},	//чт
						ApEvReads{120,"Рим., 120 зач., XVI, 1–16."},	//пт
						ApEvReads{ 97,"Рим., 97 зач., VIII, 14–21."}	//сб
					},
		std::array { ApEvReads{110,"Рим., 110 зач., XII, 6–14."},	//Неделя 6
						ApEvReads{121,"Рим., 121 зач., XVI, 17–24."},	//пн - седмица 6
						ApEvReads{122,"1 Кор., 122 зач., I, 1–9."},	//вт
						ApEvReads{127,"1 Кор., 127 зач., II, 9 – III, 8."},	//ср
						ApEvReads{129,"1 Кор., 129 зач., III, 18–23."},	//чт
						ApEvReads{130,"1 Кор., 130 зач., IV, 5-8."},	//пт
						ApEvReads{100,"Рим., 100 зач., IX, 1–5."}	//сб
					},
		std::array { ApEvReads{116,"Рим., 116 зач., XV, 1–7."},	//Неделя 7
						ApEvReads{134,"1 Кор., 134 зач., V, 9 – VI, 11."},	//пн - седмица 7
						ApEvReads{136,"1 Кор., 136 зач., VI, 20 – VII, 12."},	//вт
						ApEvReads{137,"1 Кор., 137 зач., VII, 12–24."},	//ср
						ApEvReads{138,"1 Кор., 138 зач., VII, 24–35."},	//чт
						ApEvReads{139,"1 Кор., 139 зач., VII, 35 – VIII, 7."},	//пт
						ApEvReads{108,"Рим., 108 зач., XII, 1–3."}	//сб
					},
		std::array { ApEvReads{124,"1 Кор., 124 зач., I, 10–18."},	//Неделя 8
						ApEvReads{142,"1 Кор., 142 зач., IX, 13–18."},	//пн - седмица 8
						ApEvReads{144,"1 Кор., 144 зач., X, 5–12."},	//вт
						ApEvReads{145,"1 Кор., 145 зач., X, 12–22."},	//ср
						ApEvReads{147,"1 Кор., 147 зач., X, 28 – XI, 7."},	//чт
						ApEvReads{148,"1 Кор., 148 зач., XI, 8–22."},	//пт
						ApEvReads{111,"Рим., 111 зач., XIII, 1–10."}	//сб
					},
		std::array { ApEvReads{128,"1 Кор., 128 зач., III, 9–17."},	//Неделя 9
						ApEvReads{150,"1 Кор., 150 зач., XI, 31 – XII, 6."},	//пн - седмица 9
						ApEvReads{152,"1 Кор., 152 зач., XII, 12–26."},	//вт
						ApEvReads{154,"1 Кор., 154 зач., XIII, 4 – XIV, 5."},//ср
						ApEvReads{155,"1 Кор., 155 зач., XIV, 6–19."},	//чт
						ApEvReads{157,"1 Кор., 157 зач., XIV, 26–40."},	//пт
						ApEvReads{113,"Рим., 113 зач., XIV, 6–9."}	//сб
					},
		std::array { ApEvReads{131,"1 Кор., 131 зач., IV, 9–16."},	//Неделя 10
						ApEvReads{159,"1 Кор., 159 зач., XV, 12–19."},	//пн - седмица 10
						ApEvReads{161,"1 Кор., 161 зач., XV, 29–38."},	//вт
						ApEvReads{165,"1 Кор., 165 зач., XVI, 4–12."},	//ср
						ApEvReads{167,"2 Кор., 167 зач., I, 1–7."},//чт
						ApEvReads{169,"2 Кор., 169 зач., I, 12–20."},	//пт
						ApEvReads{119,"Рим., 119 зач., XV, 30–33."}	//сб
					},
		std::array { ApEvReads{141,"1 Кор., 141 зач., IX, 2–12."},	//Неделя 11
						ApEvReads{171,"2 Кор., 171 зач., II, 3–15."},	//пн - седмица 11
						ApEvReads{172,"2 Кор., 172 зач., II, 14 – III, 3."},	//вт
						ApEvReads{173,"2 Кор., 173 зач., III, 4–11."},	//ср
						ApEvReads{175,"2 Кор., 175 зач., IV, 1–6."},	//чт
						ApEvReads{177,"2 Кор., 177 зач., IV, 13–18."},	//пт
						ApEvReads{123,"1 Кор., 123 зач., I, 3–9."}	//сб
					},
		std::array { ApEvReads{158,"1 Кор., 158 зач., XV, 1-11."},	//Неделя 12
						ApEvReads{179,"2 Кор., 179 зач., V, 10–15."},	//пн - седмица 12
						ApEvReads{180,"2 Кор., 180 зач., V, 15–21."},	//вт
						ApEvReads{182,"2 Кор., 182 зач., VI, 11–16."},	//ср
						ApEvReads{183,"2 Кор., 183 зач., VII, 1–10."},	//чт
						ApEvReads{184,"2 Кор., 184 зач., VII, 10–16."},	//пт
						ApEvReads{125,"1 Кор., 125 зач., I, 18-24."}	//сб
					},
		std::array { ApEvReads{166,"1 Кор., 166 зач., XVI, 13–24."},	//Неделя 13
						ApEvReads{186,"2 Кор., 186 зач., VIII, 7–15."},	//пн - седмица 13
						ApEvReads{187,"2 Кор., 187 зач., VIII, 16 – IX, 5."},	//вт
						ApEvReads{189,"2 Кор., 189 зач., IX, 12 – X, 7."},	//ср
						ApEvReads{190,"2 Кор., 190 зач., X, 7–18."},	//чт
						ApEvReads{192,"2 Кор., 192 зач., XI, 5–21."},	//пт
						ApEvReads{126,"1 Кор., 126 зач., II, 6–9."}	//сб
					},
		std::array { ApEvReads{170,"2 Кор., 170 зач., I, 21 – II, 4."},	//Неделя 14
						ApEvReads{195,"2 Кор., 195 зач., XII, 10–19."},	//пн - седмица 14
						ApEvReads{196,"2 Кор., 196 зач., XII, 20 – XIII, 2."},	//вт
						ApEvReads{197,"2 Кор., 197 зач., XIII, 3–13."},	//ср
						ApEvReads{198,"Гал., 198 зач., I, 1–10, 20 – II, 5."},	//чт
						ApEvReads{201,"Гал., 201 зач., II, 6–10."},	//пт
						ApEvReads{130,"1 Кор., 130 зач., IV, 1–5."}	//сб
					},
		std::array { ApEvReads{176,"2 Кор., 176 зач., IV, 6–15."},	//Неделя 15
						ApEvReads{202,"Гал., 202 зач., II, 11–16."},	//пн - седмица 15
						ApEvReads{204,"Гал., 204 зач., II, 21 – III, 7."},	//вт
						ApEvReads{207,"Гал., 207 зач., III, 15–22."},	//ср
						ApEvReads{208,"Гал., 208 зач., III, 23 - IV, 5."},	//чт
						ApEvReads{210,"Гал., 210 зач., IV, 8–21."},	//пт
						ApEvReads{132,"1 Кор., 132 зач., IV, 17 – V, 5."}	//сб
					},
		std::array { ApEvReads{181,"2 Кор., 181 зач., VI, 1–10."},	//Неделя 16
						ApEvReads{211,"Гал., 211 зач., IV, 28 – V, 10."},	//пн - седмица 16
						ApEvReads{212,"Гал., 212 зач., V, 11–21."},	//вт
						ApEvReads{214,"Гал., 214 зач., VI, 2–10."},	//ср
						ApEvReads{216,"Еф., 216 зач., I, 1–9."},	//чт
						ApEvReads{217,"Еф., 217 зач., I, 7–17."},	//пт
						ApEvReads{146,"1 Кор., 146 зач., X, 23–28."}	//сб
					},
		std::array { ApEvReads{182,"2 Кор., 182 зач., VI, 16 - VII, 1."},//Неделя 17
						ApEvReads{219,"Еф., 219 зач., I, 22 – II, 3."},	//пн - седмица 17
						ApEvReads{222,"Еф., 222 зач., II, 19 – III, 7."},	//вт
						ApEvReads{223,"Еф., 223 зач., III, 8–21."},	//ср
						ApEvReads{225,"Еф., 225 зач., IV, 14–19."},	//чт
						ApEvReads{226,"Еф., 226 зач., IV, 17–25."},	//пт
						ApEvReads{156,"1 Кор., 156 зач., XIV, 20–25."}	//сб
					},
		std::array { ApEvReads{188,"2 Кор., 188 зач., IX, 6–11."},	//Неделя 18
						ApEvReads{227,"Еф., 227 зач., IV, 25–32."},	//пн - седмица 18
						ApEvReads{230,"Еф., 230 зач., V, 20–26."},	//вт
						ApEvReads{231,"Еф., 231 зач., V, 25–33."},	//ср
						ApEvReads{232,"Еф., 232 зач., V, 33 – VI, 9."},	//чт
						ApEvReads{234,"Еф., 234 зач., VI, 18–24."},	//пт
						ApEvReads{162,"1 Кор., 162 зач., XV, 39–45."}	//сб
					},
		std::array { ApEvReads{194,"2 Кор., 194 зач., XI, 31 – XII, 9."},	//Неделя 19
						ApEvReads{235,"Флп., 235 зач., I, 1–7."},	//пн - седмица 19
						ApEvReads{236,"Флп., 236 зач., I, 8–14."},	//вт
						ApEvReads{237,"Флп., 237 зач., I, 12–20."},	//ср
						ApEvReads{238,"Флп., 238 зач., I, 20–27."},	//чт
						ApEvReads{239,"Флп., 239 зач., I, 27 – II, 4."},	//пт
						ApEvReads{164,"1 Кор., 164 зач., XV, 58 – XVI, 3."}	//сб
					},
		std::array { ApEvReads{200,"Гал., 200 зач., I, 11–19."},	//Неделя 20
						ApEvReads{241,"Флп., 241 зач., II, 12–16."},	//пн - седмица 20
						ApEvReads{242,"Флп., 242 зач., II, 16–23."},	//вт
						ApEvReads{243,"Флп., 243 зач., II, 24–30."},	//ср
						ApEvReads{244,"Флп., 244 зач., III, 1–8."},	//чт
						ApEvReads{245,"Флп., 245 зач., III, 8–19."},	//пт
						ApEvReads{168,"2 Кор., 168 зач., I, 8–11."}	//сб
					},
		std::array { ApEvReads{203,"Гал., 203 зач., II, 16–20."},	//Неделя 21
						ApEvReads{248,"Флп., 248 зач., IV, 10–23."},	//пн - седмица 21
						ApEvReads{249,"Кол., 249 зач., I, 1–2, 7–11."},	//вт
						ApEvReads{251,"Кол., 251 зач., I, 18–23."},	//ср
						ApEvReads{252,"Кол., 252 зач., I, 24–29."},	//чт
						ApEvReads{253,"Кол., 253 зач., II, 1–7."},	//пт
						ApEvReads{174,"2 Кор., 174 зач., III, 12–18."}	//сб
					},
		std::array { ApEvReads{215,"Гал., 215 зач., VI, 11–18."},	//Неделя 22
						ApEvReads{255,"Кол., 255 зач., II, 13–20."},	//пн - седмица 22
						ApEvReads{256,"Кол., 256 зач., II, 20 – III, 3."},	//вт
						ApEvReads{259,"Кол., 259 зач., III, 17 – IV, 1."},	//ср
						ApEvReads{260,"Кол., 260 зач., IV, 2–9."},	//чт
						ApEvReads{261,"Кол., 261 зач., IV, 10–18."},	//пт
						ApEvReads{178,"2 Кор., 178 зач., V, 1–10."}	//сб
					},
		std::array { ApEvReads{220,"Еф., 220 зач., II, 4–10."},	//Неделя 23
						ApEvReads{262,"1 Сол., 262 зач., I, 1–5."},	//пн - седмица 23
						ApEvReads{263,"1 Сол., 263 зач., I, 6–10."},	//вт
						ApEvReads{264,"1 Сол., 264 зач., II, 1–8."},	//ср
						ApEvReads{265,"1 Сол., 265 зач., II, 9–14."},	//чт
						ApEvReads{266,"1 Сол., 266 зач., II, 14–19."},	//пт
						ApEvReads{185,"2 Кор., 185 зач., VIII, 1–5."}	//сб
					},
		std::array { ApEvReads{221,"Еф., 221 зач., II, 14–22."},	//Неделя 24
						ApEvReads{267,"1 Сол., 267 зач., II, 20 – III, 8."},	//пн - седмица 24
						ApEvReads{268,"1 Сол., 268 зач., III, 9–13."},	//вт
						ApEvReads{269,"1 Сол., 269 зач., IV, 1–12."},	//ср
						ApEvReads{271,"1 Сол., 271 зач., V, 1–8."},	//чт
						ApEvReads{272,"1 Сол., 272 зач., V, 9–13, 24–28."},	//пт
						ApEvReads{191,"2 Кор., 191 зач., XI, 1–6."}	//сб
					},
		std::array { ApEvReads{224,"Еф., 224 зач., IV, 1–6."},	//Неделя 25
						ApEvReads{274,"2 Сол., 274 зач., I, 1–10."},	//пн - седмица 25
						ApEvReads{274,"2 Сол., 274 зач., I, 10 - II, 2."},	//вт
						ApEvReads{275,"2 Сол., 275 зач., II, 1–12."},	//ср
						ApEvReads{276,"2 Сол., 276 зач., II, 13 – III, 5."},	//чт
						ApEvReads{277,"2 Сол., 277 зач., III, 6–18."},	//пт
						ApEvReads{199,"Гал., 199 зач., I, 3–10."}	//сб
					},
		std::array { ApEvReads{229,"Еф., 229 зач., V, 8–19."},	//Неделя 26
						ApEvReads{278,"1 Тим., 278 зач., I, 1–7."},	//пн - седмица 26
						ApEvReads{279,"1 Тим., 279 зач., I, 8–14."},	//вт
						ApEvReads{281,"1 Тим., 281 зач., I, 18–20; II, 8–15."},	//ср
						ApEvReads{283,"1 Тим., 283 зач., III, 1–13."},	//чт
						ApEvReads{285,"1 Тим., 285 зач., IV, 4–8, 16."},	//пт
						ApEvReads{205,"Гал., 205 зач., III, 8–12."}	//сб
					},
		std::array { ApEvReads{233,"Еф., 233 зач., VI, 10–17."},	//Неделя 27
						ApEvReads{285,"1 Тим., 285 зач., V, 1-10."},	//пн - седмица 27
						ApEvReads{286,"1 Тим., 286 зач., V, 11–21."},	//вт
						ApEvReads{287,"1 Тим., 287 зач., V, 22 – VI, 11."},	//ср
						ApEvReads{289,"1 Тим., 289 зач., VI, 17–21."},	//чт
						ApEvReads{290,"2 Тим., 290 зач., I, 1–2, 8–18."},	//пт
						ApEvReads{213,"Гал., 213 зач., V, 22 – VI, 2."}	//сб
					},
		std::array { ApEvReads{250,"Кол., 250 зач., I, 12–18."},	//Неделя 28
						ApEvReads{294,"2 Тим., 294 зач., II, 20–26."},	//пн - седмица 28
						ApEvReads{297,"2 Тим., 297 зач., III, 16 – IV, 4."},	//вт
						ApEvReads{299,"2 Тим., 299 зач., IV, 9–22."},	//ср
						ApEvReads{300,"Тит., 300 зач., I, 5 - II, 1."},	//чт
						ApEvReads{301,"Тит., 301 зач., I, 15 – II, 10."},	//пт
						ApEvReads{218,"Еф., 218 зач., I, 16–23."}	//сб
					},
		std::array { ApEvReads{257,"Кол., 257 зач., III, 4-11."},	//Неделя 29
						ApEvReads{308,"Евр., 308 зач., III, 5–11, 17–19."},	//пн - седмица 29
						ApEvReads{310,"Евр., 310 зач., IV, 1–13."},	//вт
						ApEvReads{312,"Евр., 312 зач., V, 11 – VI, 8."},	//ср
						ApEvReads{315,"Евр., 315 зач., VII, 1–6."},	//чт
						ApEvReads{317,"Евр., 317 зач., VII, 18–25."},	//пт
						ApEvReads{220,"Еф., 220 зач., II, 11-13."}	//сб
					},
		std::array { ApEvReads{258,"Кол., 258 зач., III, 12–16."},	//Неделя 30
						ApEvReads{319,"Евр., 319 зач., VIII, 7–13."},	//пн - седмица 30
						ApEvReads{321,"Евр., 321 зач., IX, 8–10, 15–23."},	//вт
						ApEvReads{323,"Евр., 323 зач., X, 1–18."},	//ср
						ApEvReads{326,"Евр., 326 зач., X, 35 – XI, 7."},	//чт
						ApEvReads{327,"Евр., 327 зач., XI, 8, 11–16."},	//пт
						ApEvReads{228,"Еф., 228 зач., V, 1–8."}	//сб
					},
		std::array { ApEvReads{280,"1 Тим., 280 зач., I, 15-17."},	//Неделя 31
						ApEvReads{329,"Евр., 329 зач., XI, 17–23, 27–31."},//пн - седмица 31
						ApEvReads{333,"Евр., 333 зач., XII, 25–26; XIII, 22–25."},//вт
						ApEvReads{ 50,"Иак., 50 зач., I, 1-18."},	//ср
						ApEvReads{ 51,"Иак., 51 зач., I, 19-27."},	//чт
						ApEvReads{ 52,"Иак., 52 зач., II, 1–13."},	//пт
						ApEvReads{249,"Кол., 249 зач., I, 3-6."}	//сб
					},
		std::array { ApEvReads{285,"1 Тим., 285 зач., IV, 9-15."},	//Неделя 32
						ApEvReads{ 53,"Иак., 53 зач., II, 14–26."},	//пн - седмица 32
						ApEvReads{ 54,"Иак., 54 зач., III, 1–10."},	//вт
						ApEvReads{ 55,"Иак., 55 зач., III, 11 – IV, 6."},	//ср
						ApEvReads{ 56,"Иак., 56 зач., IV, 7 – V, 9."},	//чт
						ApEvReads{ 58,"1 Пет., 58 зач., I, 1–2, 10–12; II, 6–10."},//пт
						ApEvReads{273,"1 Сол., 273 зач., V, 14–23."}	//сб
					},
		std::array { ApEvReads{296,"2 Тим., 296 зач., III, 10–15."},	//Неделя 33 о мытари и фарисеи
						ApEvReads{ 59,"1 Пет., 59 зач., II, 21 – III, 9."},	//пн - седмица 33
						ApEvReads{ 60,"1 Пет., 60 зач., III, 10–22."},	//вт
						ApEvReads{ 61,"1 Пет., 61 зач., IV, 1–11."},	//ср
						ApEvReads{ 62,"1 Пет., 62 зач., IV, 12 – V, 5."},	//чт
						ApEvReads{ 64,"2 Пет., 64 зач., I, 1–10."},	//пт
						ApEvReads{293,"2 Тим., 293 зач., II, 11–19."}	//сб
					},
		std::array { ApEvReads{135,"1 Кор., 135 зач., VI, 12-20."},	//Неделя 34 о блуднем сыне
						ApEvReads{ 66,"2 Пет., 66 зач., I, 20 – II, 9."},	//пн - седмица 34
						ApEvReads{ 67,"2 Пет., 67 зач., II, 9–22."},	//вт
						ApEvReads{ 68,"2 Пет., 68 зач., III, 1–18."},	//ср
						ApEvReads{ 69,"1 Ин., 69 зач., I, 8 – II, 6."},	//чт
						ApEvReads{ 70,"1 Ин., 70 зач., II, 7–17."},	//пт
						ApEvReads{295,"2 Тим., 295 зач., III, 1–9."}	//сб
					},
		std::array { ApEvReads{140,"1 Кор., 140 зач., VIII, 8 – IX, 2."},	//Неделя 35 мясопустная
						ApEvReads{ 71,"1 Ин., 71 зач., II, 18 – III, 10."},	//пн - седмица 35
						ApEvReads{ 72,"1 Ин., 72 зач., III, 10–20."},	//вт
						ApEvReads{ 73,"1 Ин., 73 зач., III, 21 – IV, 6."},	//ср
						ApEvReads{ 74,"1 Ин., 74 зач., IV, 20 – V, 21."},	//чт
						ApEvReads{ 75,"2 Ин., 75 зач., I, 1–13."},	//пт
						ApEvReads{146,"1 Кор., 146 зач., X, 23–28."}	//сб
					},
		std::array { ApEvReads{112,"Рим., 112 зач., XIII, 11 – XIV, 4."},	//Неделя 36 сыропустная
						ApEvReads{ 76,"3 Ин., 76 зач., I, 1–15."},	//пн - седмица 36
						ApEvReads{ 77,"Иуд., 77 зач., I, 1–10."},	//вт
						ApEvReads{},	//ср
						ApEvReads{ 78,"Иуд., 78 зач., I, 11–25."},	//чт
						ApEvReads{},	//пт
						ApEvReads{115,"Рим., 115 зач., XIV, 19–26."}	//сб
					}
	};
	auto apostol_table1_get_chteniya = [](int8_t n50, int8_t dn)->ApEvReads {
		const ApEvReads & ref = apostol_table_1.at(n50).at(dn);
		return {ref.n, ref.c};
	};
	//таблица рядовых чтений на литургии из приложения богосл.евангелия. период от начала вел.поста до Троицкая суб.вкл.
	//асс.массив, где first - константа-признак даты (блок 1 - переходящие дни года)
	static const TT2 evangelie_table_2 {
		{1,    { 1, "Ин., 1 зач., I, 1–17." } },//пасха
		{2,    { 2, "Ин., 2 зач., I, 18–28." } },
		{3,    {113,"Лк., 113 зач., XXIV, 12–35."  } },
		{4,    { 4, "Ин., 4 зач., I, 35–51." } },
		{5,    { 8, "Ин., 8 зач., III, 1–15." } },
		{6,    { 7, "Ин., 7 зач., II, 12–22." } },
		{7,    {11, "Ин., 11 зач., III, 22–33." } },
		{8,    {65, "Ин., 65 зач., XX, 19–31." } },//Неделя 2, о Фоме
		{9,    { 6, "Ин., 6 зач., II, 1–11." } },
		{10,   {10, "Ин., 10 зач., III, 16–21." } },
		{11,   {15, "Ин., 15 зач., V, 17–24." } },
		{12,   {16, "Ин., 16 зач., V, 24–30." } },
		{13,   {17, "Ин., 17 зач., V, 30 – VI, 2." } },
		{14,   {19, "Ин., 19 зач., VI, 14–27." } },
		{15,   {69, "Мк., 69 зач., XV, 43–47." } },//Неделя 3, о мироносицах
		{16,   {13, "Ин., 13 зач., IV, 46–54." } },
		{17,   {20, "Ин., 20 зач., VI, 27–33." } },
		{18,   {21, "Ин., 21 зач., VI, 35–39." } },
		{19,   {22, "Ин., 22 зач., VI, 40–44." } },
		{20,   {23, "Ин., 23 зач., VI, 48–54." } },
		{21,   {52, "Ин., 52 зач., XV, 17 – XVI, 2." } },
		{22,   {14, "Ин., 14 зач., V, 1–15." } },//Неделя 4, о разслабленнем
		{23,   {24, "Ин., 24 зач., VI, 56–69." } },
		{24,   {25, "Ин., 25 зач., VII, 1–13." } },
		{25,   {26, "Ин., 26 зач., VII, 14–30." } },
		{26,   {29, "Ин., 29 зач., VIII, 12–20." } },
		{27,   {30, "Ин., 30 зач., VIII, 21–30." } },
		{28,   {31, "Ин., 31 зач., VIII, 31–42." } },
		{29,   {12, "Ин., 12 зач., IV, 5–42." } },//Неделя 5, о самаряныни
		{30,   {32, "Ин., 32 зач., VIII, 42–51." } },
		{31,   {33, "Ин., 33 зач., VIII, 51–59." } },
		{32,   {18, "Ин., 18 зач., VI, 5–14." } },
		{33,   {35, "Ин., 35 зач., IX, 39 – X, 9." } },
		{34,   {37, "Ин., 37 зач., X, 17–28." } },
		{35,   {38, "Ин., 38 зач., X, 27–38." } },
		{36,   {34, "Ин., 34 зач., IX, 1–38." } },//Неделя 6, о слепом
		{37,   {40, "Ин., 40 зач., XI, 47–57." } },
		{38,   {42, "Ин., 42 зач., XII, 19–36." } },
		{39,   {43, "Ин., 43 зач., XII, 36–47." } },
		{40,   {114,"Лк., 114 зач., XXIV, 36–53."  } },//Вознесение Господне
		{41,   {47, "Ин., 47 зач., XIV, 1–11." } },
		{42,   {48, "Ин., 48 зач., XIV, 10–21." } },
		{43,   {56, "Ин., 56 зач., XVII, 1–13." } },//Неделя 7, святых отец
		{44,   {49, "Ин., 49 зач., XIV, 27 – XV, 7." } },
		{45,   {53, "Ин., 53 зач., XVI, 2–13." } },
		{46,   {54, "Ин., 54 зач., XVI, 15–23." } },
		{47,   {55, "Ин., 55 зач., XVI, 23–33." } },
		{48,   {57, "Ин., 57 зач., XVII, 18–26." } },
		{49,   {67, "Ин., 67 зач., XXI, 15–25." } },
		{92,   {10, "Мк., 10 зач., II, 23 – III, 5." } },//Суббота первая поста
		{93,   { 5, "Ин., 5 зач., I, 43–51." } },//Неделя первая поста
		{99,   { 6, "Мк., 6 зач., I, 35–44." } },//Суббота вторая поста
		{100,  { 7, "Мк., 7 зач., II, 1–12." } },//Неделя вторая поста
		{106,  { 8, "Мк., 8 зач., II, 14–17." } },//Суббота третия поста
		{107,  {37, "Мк., 37 зач., VIII, 34 – IX, 1." } },//Неделя третия поста
		{113,  {31, "Мк., 31 зач., VII, 31–37." } },//Суббота четвертая поста
		{114,  {40, "Мк., 40 зач., IX, 17–31." } },//Неделя четвертая постa
		{120,  {35, "Мк., 35 зач., VIII, 27–31." } },//Суббота пятая поста
		{121,  {47, "Мк., 47 зач., X, 32–45." } },//Неделя пятая поста
		{127,  {39, "Ин., 39 зач., XI, 1–45." } },//Суббота шестая Лазарева
		{128,  {41, "Ин., 41 зач., XII, 1–18." } },//В неделю цветоносную
		{129,  {98, "Мф., 98 зач., XXIV, 3–35." } },//великий Понедельник
		{130,  {102,"Мф., 102 зач., XXIV, 36 - XXVI, 2." } },//великий Вторник
		{131,  {108,"Мф., 108 зач., XXVI, 6-16." } },//великую Среду
		{132,  {107,"Мф., 107 зач., XXVI, 1–20. Ин., 44 зач., XIII, 3–17. Мф., 108 зач.(от полу́), XXVI, 21–39. Лк., 109 зач., XXII, 43–45. Мф., 108 зач., XXVI, 40 – XXVII, 2." } },//великий Четверток
		{134,  {115,"Мф., 115 зач., XXVIII, 1–20." } } //великую Субботу
	};
	auto evangelie_table2_get_chteniya = [](const std::set<uint16_t>& markers)->ApEvReads {
		ApEvReads res{};
		if(markers.empty()) return res;
		std::vector<uint16_t> t_(evangelie_table_2.size());
		std::transform(evangelie_table_2.cbegin(), evangelie_table_2.cend(),
										t_.begin(),
										[](const auto& e){ return e.first; });
		auto fr1 = std::find_first_of(markers.begin(), markers.end(), t_.begin(), t_.end());
		if(fr1==markers.end()) return res;
		auto fr2 = evangelie_table_2.find(*fr1);
		if(fr2 != evangelie_table_2.end()) {
			res.n = fr2->second.n;
			res.c = fr2->second.c;
		}
		return res;
	};
	//таблица рядовых чтений на литургии из приложения богосл.апостола. период от начала вел.поста до Троицкая суб.вкл.
	//асс.массив, где first - константа-признак даты (блок 1 - переходящие дни года)
	static const TT2 apostol_table_2 {
		{1,    {  1, "Деян., 1 зач., I, 1–8." } },   //пасха
		{2,    {  2, "Деян., 2 зач., I, 12–17, 21–26." } },
		{3,    {  4, "Деян., 4 зач., II, 14–21." } },
		{4,    {  5, "Деян., 5 зач., II, 22–36." } },
		{5,    {  6, "Деян., 6 зач., II, 38–43." } },
		{6,    {  7, "Деян., 7 зач., III, 1–8." } },
		{7,    {  8, "Деян., 8 зач., III, 11–16." } },
		{8,    { 14, "Деян., 14 зач., V, 12–20." } },//Неделя 2, о Фоме
		{9,    {  9, "Деян., 9 зач., III, 19–26." } },
		{10,   { 10, "Деян., 10 зач., IV, 1–10." } },
		{11,   { 11, "Деян., 11 зач., IV, 13–22." } },
		{12,   { 12, "Деян., 12 зач., IV, 23–31." } },
		{13,   { 13, "Деян., 13 зач., V, 1–11." } },
		{14,   { 15, "Деян., 15 зач., V, 21–33." } },
		{15,   { 16, "Деян., 16 зач., VI, 1-7." } },//Неделя 3, о мироносицах
		{16,   { 17, "Деян., 17 зач., VI, 8 – VII, 5, 47–60." } },
		{17,   { 18, "Деян., 18 зач., VIII, 5–17." } },
		{18,   { 19, "Деян., 19 зач., VIII, 18–25." } },
		{19,   { 20, "Деян., 20 зач., VIII, 26–39." } },
		{20,   { 21, "Деян., 21 зач., VIII, 40 – IX, 19." } },
		{21,   { 22, "Деян., 22 зач., IX, 19–31." } },
		{22,   { 23, "Деян., 23 зач., IX, 32-42." } },//Неделя 4, о разслабленнем
		{23,   { 24, "Деян., 24 зач., X, 1–16." } },
		{24,   { 25, "Деян., 25 зач., X, 21–33." } },
		{25,   { 34, "Деян., 34 зач., XIV, 6–18." } },
		{26,   { 26, "Деян., 26 зач., X, 34–43." } },
		{27,   { 27, "Деян., 27 зач., X, 44 – XI, 10." } },
		{28,   { 29, "Деян., 29 зач., XII, 1–11." } },
		{29,   { 28, "Деян., 28 зач., XI, 19–26, 29–30." } },//Неделя 5, о самаряныни
		{30,   { 30, "Деян., 30 зач., XII, 12–17." } },
		{31,   { 31, "Деян., 31 зач., XII, 25 – XIII, 12." } },
		{32,   { 32, "Деян., 32 зач., XIII, 13–24." } },
		{33,   { 35, "Деян., 35 зач., XIV, 20–27." } },
		{34,   { 36, "Деян., 36 зач., XV, 5–34." } },
		{35,   { 37, "Деян., 37 зач., XV, 35–41." } },
		{36,   { 38, "Деян., 38 зач., XVI, 16–34." } },//Неделя 6, о слепом
		{37,   { 39, "Деян., 39 зач., XVII, 1–15." } },
		{38,   { 40, "Деян., 40 зач., XVII, 19-28." } },
		{39,   { 41, "Деян., 41 зач., XVIII, 22–28." } },
		{40,   {  1, "Деян., 1 зач., I, 1–12." } },//Вознесение Господне
		{41,   { 42, "Деян., 42 зач., XIX, 1–8." } },
		{42,   { 43, "Деян., 43 зач., XX, 7–12." } },
		{43,   { 44, "Деян., 44 зач., XX, 16-18, 28-36." } },//Неделя 7, святых отец
		{44,   { 45, "Деян., 45 зач., XXI, 8–14." } },
		{45,   { 46, "Деян., 46 зач., XXI, 26–32." } },
		{46,   { 47, "Деян., 47 зач., XXIII, 1–11." } },
		{47,   { 48, "Деян., 48 зач., XXV, 13–19." } },
		{48,   { 50, "Деян., 50 зач., XXVII, 1–44." } },
		{49,   { 51, "Деян., 51 зач., XXVIII, 1–31." } },
		{92,   {303, "Евр., 303 зач., I, 1–12." } },//Суббота первая поста
		{93,   {329, "Евр., 329 зач., XI, 24-26, 32 - XII, 2." } },//Неделя первая поста
		{99,   {309, "Евр., 309 зач., III, 12–16." } },//Суббота вторая поста
		{100,  {304, "Евр., 304 зач., I, 10 – II, 3." } },//Неделя вторая поста
		{106,  {325, "Евр., 325 зач., X, 32–38." } },//Суббота третия поста
		{107,  {311, "Евр., 311 зач., IV, 14 – V, 6." } },//Неделя третия поста
		{113,  {313, "Евр., 313 зач., VI, 9–12." } },//Суббота четвертая поста
		{114,  {314, "Евр., 314 зач., VI, 13–20." } },//Неделя четвертая постa
		{120,  {322, "Евр., 322 зач., IX, 24–28." } },//Суббота пятая поста
		{121,  {321, "Евр., 321 зач., IX, 11-14." } },//Неделя пятая поста
		{127,  {333, "Евр., 333 зач., XII, 28 - XIII, 8." } },//Суббота шестая Лазарева
		{128,  {247, "Флп., 247 зач., IV, 4-9." } },//В неделю цветоносную
		{132,  {149, "1 Кор., 149 зач., XI, 23–32." } },//великий Четверток
		{134,  {91, "Рим., 91 зач., VI, 3–11." } }//великую Субботу
	};
	auto apostol_table2_get_chteniya = [](const std::set<uint16_t>& markers)->ApEvReads {
		ApEvReads res{};
		if(markers.empty()) return res;
		std::vector<uint16_t> t_(apostol_table_2.size());
		std::transform(apostol_table_2.cbegin(), apostol_table_2.cend(),
										t_.begin(),
										[](const auto& e){ return e.first; });
		auto fr1 = std::find_first_of(markers.begin(), markers.end(), t_.begin(), t_.end());
		if(fr1==markers.end()) return res;
		auto fr2 = apostol_table_2.find(*fr1);
		if(fr2 != apostol_table_2.end()) {
			res.n = fr2->second.n;
			res.c = fr2->second.c;
		}
		return res;
	};
	//prepare second ctor parameter
	std::array<int,5> zimn_otstupka_n5;
	std::array<int,4> zimn_otstupka_n4;
	std::array<int,3> zimn_otstupka_n3;
	std::array<int,2> zimn_otstupka_n2;
	int               zimn_otstupka_n1;
	std::array<int,2> osen_otstupka;
	auto ilit = il.begin();
	zimn_otstupka_n1 = *ilit; ++ilit;
	zimn_otstupka_n2[0] = *ilit; ++ilit;
	zimn_otstupka_n2[1] = *ilit; ++ilit;
	zimn_otstupka_n3[0] = *ilit; ++ilit;
	zimn_otstupka_n3[1] = *ilit; ++ilit;
	zimn_otstupka_n3[2] = *ilit; ++ilit;
	zimn_otstupka_n4[0] = *ilit; ++ilit;
	zimn_otstupka_n4[1] = *ilit; ++ilit;
	zimn_otstupka_n4[2] = *ilit; ++ilit;
	zimn_otstupka_n4[3] = *ilit; ++ilit;
	zimn_otstupka_n5[0] = *ilit; ++ilit;
	zimn_otstupka_n5[1] = *ilit; ++ilit;
	zimn_otstupka_n5[2] = *ilit; ++ilit;
	zimn_otstupka_n5[3] = *ilit; ++ilit;
	zimn_otstupka_n5[4] = *ilit; ++ilit;
	osen_otstupka   [0] = *ilit; ++ilit;
	osen_otstupka   [1] = *ilit; ++ilit;
	//ctor internal data structures
	struct DayData {
		int8_t dn;
		int8_t glas;
		int8_t n50;
		ApEvReads apostol;
		ApEvReads evangelie;
		std::set<uint16_t> day_markers;
		DayData() : dn{-1}, glas{-1}, n50{-1} {}
		DayData(int8_t x) : dn{x}, glas{-1}, n50{-1} {}
	};
	std::map<ShortDate, DayData> days;
	std::multimap<uint16_t, ShortDate> markers;
	const auto pasha_date = pasha_calc(y);
	const auto pasha_date_pred = pasha_calc(y-1);
	auto is_visokos = [](int g) { return (g%4)==0; };
	const bool b = is_visokos(y);
	const bool b1 = is_visokos(y-1);
	ShortDate ned_pr, nachalo_posta, t1, t2, t3;
	ShortDate dd {pasha_date};
	int i = 0, j = 0, glas = 8;
	bool f = false;
	//карта дней недели пред. года
	std::map<ShortDate, int8_t> dn_prev;
	//функц.возвращает кол-во дней в месяце или -1
	auto get_days_inmonth_ = [](int8_t month, bool leap) -> int8_t {
		int8_t k{-1};
		switch(month) {
			case 1: { k = 31; }
					break;
			case 2: { k = (leap ? 29 : 28); }
					break;
			case 3: { k = 31; }
					break;
			case 4: { k = 30; }
					break;
			case 5: { k = 31; }
					break;
			case 6: { k = 30; }
					break;
			case 7: { k = 31; }
					break;
			case 8: { k = 31; }
					break;
			case 9: { k = 30; }
					break;
			case 10:{ k = 31; }
					break;
			case 11:{ k = 30; }
					break;
			case 12:{ k = 31; }
		};
		return k;
	};
	//функц.возвращает дату увеличенную на кол-во дней доконца года
	//если инкремент переходит через конец года, возвращает исходную дату
	//date.first - месяц; date.second - день
	auto increment_date_ = [&get_days_inmonth_](ShortDate date, int days, bool visokos) -> ShortDate {
		ShortDate r {date};
		ShortDate l {date};
		if(days<1) { return l; }
		if(date.first<1 || date.first>12)  { return l; }
		int u = get_days_inmonth_(date.first, visokos);
		if(date.second<1 || date.second>u) { return l; }
		r.second += days;
		while (r.second>u) {
			r.second = r.second - u;
			r.first++;
			if(r.first>12) return l;
			u = get_days_inmonth_(r.first, visokos);
		}
		return r;
	};
	//функц.возвращает дату уменьшенную на кол-во дней до начала года
	//если декремент переходит через начало года, возвращает исходную дату
	//date.first - месяц; date.second - день
	auto decrement_date_ = [&get_days_inmonth_](ShortDate date, int days, bool visokos) -> ShortDate {
		ShortDate r {date};
		ShortDate l {date};
		if(days<1) { return l; }
		if(date.first<1 || date.first>12)  { return l; }
		int u = get_days_inmonth_(date.first, visokos);
		if(date.second<1 || date.second>u) { return l; }
		r.second = r.second - days;
		while (r.second < 1) {
			r.first--;
			if(r.first<1) return l;
			u = get_days_inmonth_(r.first, visokos);
			r.second = u + r.second;
		}
		return r;
	};
	//функц.создание карты дней недели указанного года в формате:
	//key - дата; key.first - месяц; key.second - день
	//value - деньнедели; 0-вс, 1-пн, 2-вт, 3-ср, 4-чт, 5-пт, 6-сб.
	auto create_days_map_ = [&increment_date_, &decrement_date_, &is_visokos](int y) -> std::optional<std::map<ShortDate, int8_t>> {
		if(y<1) return std::nullopt;
		const bool b = is_visokos(y);
		const auto pasha_date = pasha_calc(y) ;
		ShortDate data1, data2;
		std::map<ShortDate, int8_t> result;
		int i = 0;
		while (i<7) {
			data1 = increment_date_(pasha_date, i, b);
			data2 = increment_date_(data1, 7, b);
			result.insert({data1, i});
			while (data1 != data2) {
				result.insert({data2, i});
				data1 = data2;
				data2 = increment_date_(data1, 7, b);
			}
			data1 = decrement_date_(pasha_date, 7-i, b);
			if(data1 != pasha_date) result.insert({data1, i});
			data2 = decrement_date_(data1, 7, b);
			while (data2 != data1) {
				result.insert({data2, i});
				data1 = data2;
				data2 = decrement_date_(data1, 7, b);
			}
			i++;
		}
		return result;
	};
	//функц.установка признака для даты
	auto add_marker_for_date_ = [&days, &markers](const ShortDate& d, const uint16_t m){
		if(auto fr = days.find(d); fr!=days.end()) {
			fr->second.day_markers.insert(m);
			markers.insert({m, d});
		}
	};
	//функц.установка нескольких признакoB для даты
	auto add_markers_for_date_ = [&days, &markers](const ShortDate& d, std::initializer_list<uint16_t> l){
		if(auto fr = days.find(d); fr!=days.end()) {
			for(auto i: l) {
				fr->second.day_markers.insert(i);
				markers.insert({i, d});
			}
		}
	};
	//функц.поиск дня недели
	auto get_dn_ = [&days](const ShortDate& d)->int8_t{
		if(auto e = days.find(d); e!=days.end()) return e->second.dn;
		else return -1;
	};
	auto get_dn_prev_year = [&dn_prev](const ShortDate& d)->int8_t{
		if(auto e = dn_prev.find(d); e!=dn_prev.end()) return e->second;
		else return -1;
	};
	//функц.проверки даты на признак
	auto check_date_ = [&days](const ShortDate& d, const uint16_t m){
		if(auto fr = days.find(d); fr!=days.end()) {
			return fr->second.day_markers.contains(m);
		} else {
			return false;
		}
	};
	//функц.поиск даты попризнаку
	auto get_date_ = [&markers](const uint16_t m)->ShortDate {
		if(auto fr = markers.find(m); fr!=markers.end()) {
			return fr->second;
		} else {
			return ShortDate(-1, -1);
		}
	};
	//функц.установка гласа для даты
	auto set_glas_ = [&days](const ShortDate& d, const int8_t glas){
		if(auto fr = days.find(d); fr!=days.end()) {
			fr->second.glas = glas;
		}
	};
	//функц.установка евангелия для даты
	auto set_evangelie_ = [&days](const ShortDate& d, const ApEvReads& ev){
		if(auto fr = days.find(d); fr!=days.end()) {
			fr->second.evangelie = ev;
		}
	};
	//функц.установка апостола для даты
	auto set_apostol_ = [&days](const ShortDate& d, const ApEvReads& ap){
		if(auto fr = days.find(d); fr!=days.end()) {
			fr->second.apostol = ap;
		}
	};
	//функц.установка номер по пятидесятнице для даты
	auto set_n50_ = [&days](const ShortDate& d, const int8_t n){
		if(auto fr = days.find(d); fr!=days.end()) {
			fr->second.n50 = n;
		}
	};
	//функц.поиск номер по пятидесятнице для даты
	auto get_n50_ = [&days](const ShortDate& d)->int8_t{
		if(auto e = days.find(d); e!=days.end()) return e->second.n50;
		else return -1;
	};
	//создание карт дней недели всего года
	if(auto x = create_days_map_(y)) {
		std::transform(x->cbegin(), x->cend(), std::inserter(days, days.end()), [](const auto& e){
			return std::make_pair(e.first, DayData{e.second});
		});
	}
	if(auto x = create_days_map_(y-1)) {
		dn_prev = std::move(*x);
	}
	//расчет дат непереходящих праздников
	for(auto it{stable_dates.begin()}; it!=stable_dates.end(); it=std::next(it,3)) {
		add_marker_for_date_(make_pair(*std::next(it), *std::next(it,2)), *it);
	}
	for(const auto& x: svyatki_dates) add_marker_for_date_(x, full7_svyatki);
	for( auto x=make_pair(11,15), y=make_pair(12,25); x<y; x=increment_date_(x,1,b) ) {
		add_marker_for_date_(x, post_rojd);
	}
	for( auto x=make_pair(8,1), y=make_pair(8,15); x<y; x=increment_date_(x,1,b) ) {
		add_marker_for_date_(x, post_usp);
	}
	//расчет дат от пасхи до дня всех всятых
	add_markers_for_date_(dd, {pasha, full7_pasha});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {svetlaya1, full7_pasha});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {svetlaya2, full7_pasha});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {svetlaya3, full7_pasha});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {svetlaya4, full7_pasha});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {svetlaya5, full7_pasha});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {svetlaya6, full7_pasha});
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, ned2_popashe);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s2popashe_1);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s2popashe_2);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s2popashe_3);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s2popashe_4);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s2popashe_5);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s2popashe_6);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, ned3_popashe);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s3popashe_1);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s3popashe_2);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s3popashe_3);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s3popashe_4);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s3popashe_5);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s3popashe_6);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, ned4_popashe);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s4popashe_1);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s4popashe_2);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s4popashe_3);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s4popashe_4);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s4popashe_5);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s4popashe_6);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, ned5_popashe);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s5popashe_1);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s5popashe_2);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s5popashe_3);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s5popashe_4);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s5popashe_5);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s5popashe_6);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, ned6_popashe);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s6popashe_1);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s6popashe_2);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s6popashe_3);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s6popashe_4);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s6popashe_5);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s6popashe_6);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, ned7_popashe);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s7popashe_1);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s7popashe_2);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s7popashe_3);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s7popashe_4);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s7popashe_5);
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, s7popashe_6);
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {ned8_popashe, full7_troica});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {s1po50_1, full7_troica});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {s1po50_2, full7_troica});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {s1po50_3, full7_troica});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {s1po50_4, full7_troica});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {s1po50_5, full7_troica});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {s1po50_6, full7_troica});
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, ned1_po50);
	for( auto x=increment_date_(dd,1,b), y=make_pair(6,29); x<y; x=increment_date_(x,1,b) ) {
		add_marker_for_date_(x, post_petr);
	}
	//1-я пятница Петрова поста - Прп. Варлаама Хутынского
	dd = increment_date_(dd, 5, b);
	add_marker_for_date_(dd, pyatnica9_popashe);
	//всех святых, в земле Русской просиявших
	dd = increment_date_(dd, 2, b);
	add_marker_for_date_(dd, ned2_po50);
	//всех мучеников по взятии Царяграда пострадавших
	dd = increment_date_(dd, 7, b);
	add_marker_for_date_(dd, ned3_po50);
	//дня преподобных отцов Псково-Печерских
	dd = increment_date_(dd, 7, b);
	add_marker_for_date_(dd, ned4_po50);
	//Соборa Валаамских святых
	dd = make_pair(8,7);
	do {
		i = get_dn_(dd);
		if(i==0) {
			add_marker_for_date_(dd, ned_popreobrajenii);
			break;
		}
		dd = increment_date_(dd, 1, b);
	} while (true);
	//Перенесение мощей блгвв. кн. Петра и Февронии
	dd = make_pair(9,6);
	do {
		i = get_dn_(dd);
		if(i==0) {
			add_marker_for_date_(dd, ned_pered6sent);
			break;
		}
		dd = decrement_date_(dd, 1, b);
	} while (true);
	//Суббота перед Воздви́жение
	dd = make_pair(9,13);
	do {
		i = get_dn_(dd);
		if(i==6) {
			add_marker_for_date_(dd, sub_pered14sent);
			break;
		}
		dd = decrement_date_(dd, 1, b);
	} while (true);
	//неделя перед Воздви́жение
	dd = make_pair(9,13);
	do {
		i = get_dn_(dd);
		if(i==0) {
			add_marker_for_date_(dd, ned_pered14sent);
			break;
		}
		dd = decrement_date_(dd, 1, b);
	} while (true);
	//Суббота после Воздви́жение
	dd = make_pair(9,15);
	do {
		i = get_dn_(dd);
		if(i==6) {
			add_marker_for_date_(dd, sub_po14sent);
			break;
		}
		dd = increment_date_(dd, 1, b);
	} while (true);
	//неделя после Воздви́жение
	dd = make_pair(9,15);
	do {
		i = get_dn_(dd);
		if(i==0) {
			add_marker_for_date_(dd, ned_po14sent);
			break;
		}
		dd = increment_date_(dd, 1, b);
	} while (true);
	//Святых отец 7 вселенск соборa
	dd = make_pair(10,11);
	i = get_dn_(dd);
	switch (i) {
		case 0: {
			add_marker_for_date_(dd, ned_pod11okt);
		}
		break;
		case 1: {}
		case 2: {}
		case 3: {
			do {
				dd = decrement_date_(dd, 1, b);
				i = get_dn_(dd);
				if(i==0) {
					add_marker_for_date_(dd, ned_pod11okt);
					break;
				}
			} while(true);
		}
		break;
		case 4: {}
		case 5: {}
		case 6: {
			do {
				dd = increment_date_(dd, 1, b);
				i = get_dn_(dd);
				if(i==0) {
					add_marker_for_date_(dd, ned_pod11okt);
					break;
				}
			} while(true);
		}
		break;
		default: {}
	};
	//дмитриевская родительская суббота
	dd = make_pair(10,25);
	do {
		i = get_dn_(dd);
		if(i==6 && dd.second!=22) {
			add_marker_for_date_(dd, sub_dmitry);
			break;
		}
		dd = decrement_date_(dd, 1, b);
	} while (true);
	//собор безсребреников
	dd = make_pair(11,1);
	i = get_dn_(dd);
	switch (i) {
		case 0: {
			add_marker_for_date_(dd, ned_pod1noyabr);
		}
		break;
		case 1: { }
		case 2: { }
		case 3: {
			do {
				dd = decrement_date_(dd, 1, b);
				i = get_dn_(dd);
				if(i==0) {
					add_marker_for_date_(dd, ned_pod1noyabr);
					break;
				}
			} while(true);
		}
		break;
		case 4: { }
		case 5: { }
		case 6: {
			do {
				dd = increment_date_(dd, 1, b);
				i = get_dn_(dd);
				if(i==0) {
					add_marker_for_date_(dd, ned_pod1noyabr);
					break;
				}
			} while(true);
		}
		break;
		default: { }
	};
	//неделя св.праотец от11до17 дек.
	dd = make_pair(12,24);
	do {
		i = get_dn_(dd);
		if(i==0) {
			ned_pr = dd;
			break;
		}
		dd = decrement_date_(dd, 1, b);
	} while (true);
	dd = decrement_date_(dd, 1, b);
	do {
		i = get_dn_(dd);
		if(i==0) {
			add_marker_for_date_(dd, ned_praotec);
			break;
		}
		dd = decrement_date_(dd, 1, b);
	} while (true);
	//Суббота перед рождеством
	dd = make_pair(12,24);
	do {
		i = get_dn_(dd);
		if(i==6) {
			add_marker_for_date_(dd, sub_peredrojd);
			break;
		}
		dd = decrement_date_(dd, 1, b);
	} while (true);
	//нед.св.отец перед рождеством от 18до24 дек.
	add_marker_for_date_(ned_pr, ned_peredrojd);
	//Суббота+воскресение после рождества
	i = get_dn_(make_pair(12,25));
	switch(i) {
		case 6: { }
		case 0: {
			add_marker_for_date_(make_pair(12,31), sub_porojdestve);
			add_marker_for_date_(make_pair(12,26), ned_porojdestve);
		}
		break;
		case 1: {
			add_marker_for_date_(make_pair(12,30), sub_porojdestve);
			add_marker_for_date_(make_pair(12,31), ned_porojdestve);
		}
		break;
		case 2: {
			add_marker_for_date_(make_pair(12,29), sub_porojdestve);
			add_marker_for_date_(make_pair(12,30), ned_porojdestve);
		}
		break;
		case 3: {
			add_marker_for_date_(make_pair(12,28), sub_porojdestve);
			add_marker_for_date_(make_pair(12,29), ned_porojdestve);
		}
		break;
		case 4: {
			add_marker_for_date_(make_pair(12,27), sub_porojdestve);
			add_marker_for_date_(make_pair(12,28), ned_porojdestve);
		}
		break;
		case 5: {
			add_marker_for_date_(make_pair(12,26), sub_porojdestve);
			add_marker_for_date_(make_pair(12,27), ned_porojdestve);
		}
		break;
		default: {}
	};
	//неделя мытаря и фарисея
	dd = decrement_date_(pasha_date, 70, b);
	add_markers_for_date_(dd, {ned_mitar_ifaris, full7_mitar});
	for(auto x = increment_date_(dd, 1, b), y = increment_date_(dd, 7, b); x<y; x = increment_date_(x, 1, b)) {
		add_marker_for_date_(x, full7_mitar);
	}
	//неделя о блудномсыне
	dd = increment_date_(dd, 7, b);
	add_marker_for_date_(dd, ned_obludnom);
	//вселенская родительская суббота, мясопустная
	dd = increment_date_(dd, 6, b);
	add_marker_for_date_(dd, sub_myasopust);
	t1 = dd;
	//от недели мясопустной до вел.субботы
	dd = increment_date_(dd, 1, b);
	add_marker_for_date_(dd, ned_myasopust);
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {sirnaya1, full7_sirn});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {sirnaya2, full7_sirn});
	dd = increment_date_(dd, 1, b);
	t2 = dd;
	add_markers_for_date_(dd, {sirnaya3, full7_sirn});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {sirnaya4, full7_sirn});
	dd = increment_date_(dd, 1, b);
	t3 = dd;
	add_markers_for_date_(dd, {sirnaya5, full7_sirn});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {sirnaya6, full7_sirn});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {ned_siropust, full7_sirn});
	dd = increment_date_(dd, 1, b);
	nachalo_posta = dd;
	add_markers_for_date_(dd, {vel_post_d1n1, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d2n1, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d3n1, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d4n1, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d5n1, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d6n1, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d0n2, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d1n2, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d2n2, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d3n2, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d4n2, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d5n2, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d6n2, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d0n3, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d1n3, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d2n3, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d3n3, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d4n3, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d5n3, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d6n3, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d0n4, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d1n4, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d2n4, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d3n4, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d4n4, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d5n4, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d6n4, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d0n5, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d1n5, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d2n5, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d3n5, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d4n5, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d5n5, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d6n5, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d0n6, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d1n6, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d2n6, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d3n6, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d4n6, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d5n6, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d6n6, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d0n7, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d1n7, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d2n7, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d3n7, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d4n7, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d5n7, post_vel});
	dd = increment_date_(dd, 1, b);
	add_markers_for_date_(dd, {vel_post_d6n7, post_vel});
	//Суббота пред Богоявлением u неделя пред Богоявлением
	i = get_dn_prev_year(make_pair(12,25));
	switch(i) {
		case 2: {
			add_marker_for_date_(make_pair(1,5), sub_peredbogoyav_m1);
			add_marker_for_date_(make_pair(1,1), ned_peredbogoyav);
		}
		break;
		case 1: { }
		case 0: {
			add_marker_for_date_(make_pair(1,1), ned_peredbogoyav);
		}
		break;
		case 3: {
			add_marker_for_date_(make_pair(1,4), sub_peredbogoyav_m1);
			add_marker_for_date_(make_pair(1,5), ned_peredbogoyav);
		}
		break;
		case 4: {
			add_marker_for_date_(make_pair(1,3), sub_peredbogoyav_m1);
			add_marker_for_date_(make_pair(1,4), ned_peredbogoyav);
		}
		break;
		case 5: {
			add_marker_for_date_(make_pair(1,2), sub_peredbogoyav_m1);
			add_marker_for_date_(make_pair(1,3), ned_peredbogoyav);
		}
		break;
		case 6: {
			add_marker_for_date_(make_pair(1,1), sub_peredbogoyav_m1);
			add_marker_for_date_(make_pair(1,2), ned_peredbogoyav);
		}
		break;
		default: {}
	};
	i = get_dn_(make_pair(12,25));
	if(i==0) add_marker_for_date_(make_pair(12,31), sub_peredbogoyav_m12);
	if(i==1) add_marker_for_date_(make_pair(12,30), sub_peredbogoyav_m12);
	//Суббота пo Богоявление
	dd = make_pair(1,7);
	do {
		i = get_dn_(dd);
		if(i==6) {
			add_marker_for_date_(dd, sub_pobogoyav);
			break;
		}
		dd = increment_date_(dd, 1, b);
	} while (true);
	//неделя пo Богоявление
	dd = make_pair(1,7);
	do {
		i = get_dn_(dd);
		if(i==0) {
			add_marker_for_date_(dd, ned_pobogoyav);
			break;
		}
		dd = increment_date_(dd, 1, b);
	} while (true);
	//собор новомучеников русской церкви
	dd = make_pair(1,25);
	i = get_dn_(dd);
	switch (i) {
		case 0: {
			add_marker_for_date_(dd, ned_pod25yanv);
		}
		break;
		case 1: { }
		case 2: { }
		case 3: {
			do {
				dd = decrement_date_(dd, 1, b);
				i = get_dn_(dd);
				if(i==0) {
					add_marker_for_date_(dd, ned_pod25yanv);
					break;
				}
			} while(true);
		}
		break;
		case 4: { }
		case 5: { }
		case 6: {
			do {
				dd = increment_date_(dd, 1, b);
				i = get_dn_(dd);
				if(i==0) {
					add_marker_for_date_(dd, ned_pod25yanv);
					break;
				}
			} while(true);
		}
		break;
		default: {}
	};
	//собор 3-x святителей
	dd = make_pair(1, 30);
	if(dd==t1 || dd==t2 || dd==t3) dd = make_pair(1, 29);
	add_marker_for_date_(dd, sobor_3sv);
	//Сре́тение Господа Бога и Спаса нашего Иисуса Христа
	dd = make_pair(2, 2);
	if(dd>=nachalo_posta) dd = decrement_date_(nachalo_posta, 1, b);
	add_marker_for_date_(dd, sretenie);
	if(dd==t1) {
		// если сретение и вселенская родительская суббота выпали на один день
		// то перемещаем субботу на неделю раньше
		if(auto fr = days.find(t1); fr!=days.end()) {
			fr->second.day_markers.erase(sub_myasopust);
			markers.erase(sub_myasopust);
			t1 = decrement_date_(t1, 1, b);
			do {
				i = get_dn_(t1);
				if(i==6) {
					add_marker_for_date_(t1, sub_myasopust);
					break;
				}
				t1 = decrement_date_(t1, 1, b);
			} while (true);
		}
	}
	//Предпразднство Сре́тения
	if(dd != make_pair(2, 1)) {
		dd = make_pair(2, 1);
		if(dd==t1) dd = decrement_date_(dd, 1, b);
		add_marker_for_date_(dd, sretenie_predpr);
	}
	//отдание праздника Сре́тения
	dd = get_date_(sretenie);
	t3 = make_pair(2, 9);
	t1 = get_date_(ned_obludnom);
	t2 = increment_date_(t1, 2, b);
	if(dd>=t1 && dd<=t2) {
		t3 = increment_date_(t1, 5, b);
	}
	t1 = increment_date_(t1, 3, b);
	t2 = increment_date_(t1, 3, b);
	if(dd>=t1 && dd<=t2) {
		t3 = get_date_(sirnaya2);
	}
	t1 = get_date_(ned_myasopust);
	t2 = get_date_(sirnaya1);
	if(dd>=t1 && dd<=t2) {
		t3 = get_date_(sirnaya4);
	}
	t1 = get_date_(sirnaya2);
	t2 = get_date_(sirnaya3);
	if(dd>=t1 && dd<=t2) {
		t3 = get_date_(sirnaya6);
	}
	t1 = get_date_(sirnaya4);
	t2 = get_date_(sirnaya6);
	if(dd>=t1 && dd<=t2) {
		t3 = get_date_(ned_siropust);
	}
	if(!check_date_(dd, ned_siropust)) {
		if(check_date_(t3, sub_myasopust)) t3 = decrement_date_(t3, 1, b);
		add_marker_for_date_(t3, sretenie_otdanie);
	}
	//Попразднствa Сретения Господня
	t3 = get_date_(sretenie_otdanie);
	t1 = increment_date_(dd, 1, b);
	t2 = t1;
	i = 1;
	if(t3!=make_pair(-1,-1) && t3!=t1) {
		do {
			if(check_date_(t2, sub_myasopust)) {
				t2 = increment_date_(t2, 1, b);
				if(t2>=t3) break;
			}
			switch(i) {
				case 1: { add_marker_for_date_(t2, sretenie_poprazd1); }
				break;
				case 2: { add_marker_for_date_(t2, sretenie_poprazd2); }
				break;
				case 3: { add_marker_for_date_(t2, sretenie_poprazd3); }
				break;
				case 4: { add_marker_for_date_(t2, sretenie_poprazd4); }
				break;
				case 5: { add_marker_for_date_(t2, sretenie_poprazd5); }
				break;
				case 6: { add_marker_for_date_(t2, sretenie_poprazd6); }
				break;
				default:{ }
			};
			t2 = increment_date_(t2, 1, b);
			i++;
		} while (t2!=t3);
	}
	//Первое и второе Обре́тение главы Иоанна Предтечи
	dd = make_pair(2, 24);
	if( check_date_(dd, sub_myasopust) || check_date_(dd, sirnaya3) ||
		  check_date_(dd, sirnaya5) || check_date_(dd, vel_post_d1n1) )
	{
		dd = make_pair(2, 23);
	}
	t1 = get_date_(vel_post_d2n1);
	t2 = get_date_(vel_post_d5n1);
	if(dd>=t1 && dd<=t2) dd = get_date_(vel_post_d6n1);
	add_marker_for_date_(dd, obret_gl_ioanna12);
	//Святых сорока́ мучеников, в Севасти́йском е́зере мучившихся
	dd = make_pair(3, 9);
	if(check_date_(dd, vel_post_d3n4)) dd = make_pair(3, 8);
	if(check_date_(dd, vel_post_d4n5)) dd = make_pair(3, 7);
	if(check_date_(dd, vel_post_d6n5)) dd = make_pair(3, 10);
	t1 = get_date_(vel_post_d1n1);
	t2 = get_date_(vel_post_d5n1);
	if(dd>=t1 && dd<=t2) dd = get_date_(vel_post_d6n1);
	add_marker_for_date_(dd, muchenik_40);
	//предпразднество Благовещ́ение Пресвято́й Богоро́дицы
	dd = make_pair(3, 24);
	t1 = get_date_(vel_post_d1n7);
	t2 = make_pair(3, 25);
	if(t2<t1) {
		if(check_date_(dd, vel_post_d6n6)) dd = make_pair(3, 22);
		if(check_date_(dd, vel_post_d4n5)) dd = make_pair(3, 23);
		if(check_date_(dd, vel_post_d2n5)) dd = make_pair(3, 23);
		add_marker_for_date_(dd, blag_predprazd);
	}
	//отдание праздника Благовещ́ение
	dd = make_pair(3, 26);
	t1 = get_date_(vel_post_d6n6);
	if(dd<t1) {
		add_marker_for_date_(dd, blag_otdanie);
	}
	//Вмч. Гео́ргия Победоно́сца. Мц. царицы Александры
	dd = make_pair(4, 23);
	t1 = get_date_(vel_post_d1n7);
	t2 = get_date_(pasha);
	if(dd>=t1 && dd<=t2) {
		dd = get_date_(svetlaya1);
	}
	add_marker_for_date_(dd, georgia_pob);
	//третье Обре́тение главы Иоанна Предтечи.
	dd = make_pair(5, 25);
	t1 = get_date_(s7popashe_6);
	t2 = get_date_(ned1_po50);
	if(dd==t1 || dd==t2) dd = make_pair(5, 23);
	if(check_date_(dd, s1po50_1)) dd = make_pair(5, 26);
	if(check_date_(dd, ned8_popashe)) dd = make_pair(5, 22);
	add_marker_for_date_(dd, obret_gl_ioanna3);
	//Собор Тверских святых
	dd = make_pair(6, 30);
	do {
		i = get_dn_(dd);
		if(i==0) {
			add_marker_for_date_(dd, ned_popetraipavla);
			break;
		}
		dd = increment_date_(dd, 1, b);
	} while (true);
	//Святых отец 6-и вселенских соборов
	dd = make_pair(7, 16);
	i = get_dn_(dd);
	switch (i) {
		case 0: {
			add_marker_for_date_(dd, ned_pod16iulya);
		}
		break;
		case 1: { }
		case 2: { }
		case 3: {
			do {
				dd = decrement_date_(dd, 1, b);
				i = get_dn_(dd);
				if(i==0) {
					add_marker_for_date_(dd, ned_pod16iulya);
					break;
				}
			} while(true);
		}
		break;
		case 4: { }
		case 5: { }
		case 6: {
			do {
				dd = increment_date_(dd, 1, b);
				i = get_dn_(dd);
				if(i==0) {
					add_marker_for_date_(dd, ned_pod16iulya);
					break;
				}
			} while(true);
		}
		break;
		default: { }
	};
	//Собор Кемеровских святых
	dd = make_pair(8, 17);
	do {
		i = get_dn_(dd);
		if(i==0) {
			add_marker_for_date_(dd, ned_pered18avg);
			break;
		}
		dd = decrement_date_(dd, 1, b);
	} while (true);
	//расчет Двунадесятые переходящие праздники
	add_marker_for_date_(pasha_date,                   dvana10_per_prazd);
	add_marker_for_date_(get_date_(vel_post_d0n7),     dvana10_per_prazd);
	add_marker_for_date_(get_date_(s6popashe_4),       dvana10_per_prazd);
	add_marker_for_date_(get_date_(ned8_popashe),      dvana10_per_prazd);
	//расчет Двунадесятые непереходящие праздники
	add_marker_for_date_(get_date_(m1d6),     dvana10_nep_prazd);
	add_marker_for_date_(get_date_(sretenie), dvana10_nep_prazd);
	add_marker_for_date_(get_date_(m3d25),    dvana10_nep_prazd);
	add_marker_for_date_(get_date_(m8d6),     dvana10_nep_prazd);
	add_marker_for_date_(get_date_(m8d15),    dvana10_nep_prazd);
	add_marker_for_date_(get_date_(m9d8),     dvana10_nep_prazd);
	add_marker_for_date_(get_date_(m9d14),    dvana10_nep_prazd);
	add_marker_for_date_(get_date_(m11d21),   dvana10_nep_prazd);
	add_marker_for_date_(get_date_(m12d25),   dvana10_nep_prazd);
	//расчет великие праздники
	add_marker_for_date_(get_date_(m1d1),    vel_prazd);
	add_marker_for_date_(get_date_(m6d24),   vel_prazd);
	add_marker_for_date_(get_date_(m6d29),   vel_prazd);
	add_marker_for_date_(get_date_(m8d29),   vel_prazd);
	add_marker_for_date_(get_date_(m10d1),   vel_prazd);
	//расчет гласов период от субб. лазарева до нед. всех святых
	t1 = get_date_(vel_post_d6n6);//субб лазарева
	t2 = get_date_(ned1_po50);//нед всех святых
	dd = t1;
	do {
		set_glas_(dd, -1); //неопределенный глас: -1
		dd = increment_date_(dd, 1, b);
	} while(dd<=t2);
	//расчет гласов период от начала петрова поста до конца года
	dd = increment_date_(t2, 1, b);
	j = get_dn_(dd);
	do {
		do {
			set_glas_(dd, glas);
			t3 = dd;
			dd = increment_date_(dd, 1, b);
			if(dd==t3) { f = true; break; }
			j = get_dn_(dd);
		} while(j>0);
		if(f || j<0) break;
		glas++;
		if(glas>8) glas = 1;
	} while(true);
	//расчет гласов период от начала года до субб. лазарева
	t1 = pasha_date_pred;
	dd = increment_date_(t1, 57, b1);
	f = false;
	j = 1;
	glas = 8;
	do {
		do {
			t3 = dd;
			dd = increment_date_(dd, 1, b1);
			if(dd==t3) { f = true; break; }
			j = get_dn_prev_year(dd);
		} while(j>0);
		if(f || j<0) break;
		glas++;
		if(glas>8) glas = 1;
	} while(true);
	dd = make_pair(1, 1);
	j = get_dn_(dd);
	t1 = get_date_(vel_post_d6n6);
	if(j<1) {
		glas++;
		if(glas>8) glas = 1;
	}
	f = false;
	do {
		do {
			set_glas_(dd, glas);
			dd = increment_date_(dd, 1, b);
			if(dd==t1) { f = true; break; }
			j = get_dn_(dd);
		} while(j>0);
		if(f || j<0) break;
		glas++;
		if(glas>8) glas = 1;
	} while(true);
	//расчет календарный номер по пятидесятнице каждого дня года
	t1 = increment_date_(pasha_date_pred, 49, b1);
	i = 0;
	while(true) {
		t2 = increment_date_(t1, 1, b1);
		if(t2!=t1) { t1 = t2; }
		else       { break; }
		if(get_dn_prev_year(t1)==1) i++; //i = номер для 31дек. пред. года
	}
	t1 = make_pair(1, 1);
	if(get_dn_(t1)==1) i++;//i = номер для 1янв. текущ. года
	nachalo_posta = get_date_(vel_post_d1n1);//началo вел.поста
	dd = get_date_(ned8_popashe);//пятидесятницa
	while(true) {
		if(t1<nachalo_posta) {
			set_n50_(t1, i);
		} else if(t1>=nachalo_posta && t1<dd) {
			//для периода от начала вел.поста до тр.род.субб. вкл.
			set_n50_(t1, -1);
		} else if(t1==dd) {
			//для пятидесятницы
			set_n50_(t1, 0);
			i = 0;
		} else {
			//для периода от пятидесятницы до конца года
			set_n50_(t1, i);
		}
		t2 = increment_date_(t1, 1, b);
		if(t2!=t1) { t1 = t2; }
		else       { break; }
		if(get_dn_(t1)==1) i++;
	}
	//расчет рядовые чтения евангелия на литургии
	i = j = 0;
	std::vector<int> v, w, v1, w1;//контейнеры для номеров доб.седмиц и недель
	t1 = make_pair(1, 1);
	t3 = make_pair(9, 15);
	dd = get_date_(ned_mitar_ifaris);
	auto ddd = increment_date_(pasha_date_pred, 49, b1);//пятидесятницa пред. года
	auto d2 {get_date_(ned_pobogoyav)};
	auto mf7  {increment_date_(dd,7,b)};
	auto mf14 {increment_date_(dd,14,b)};
	auto mf21 {increment_date_(dd,21,b)};
	auto ned_po_vozdv {get_date_(ned_po14sent)};
	auto dd1 {decrement_date_(ned_po_vozdv, 14, b)};
	auto dd2 {decrement_date_(ned_po_vozdv,  7, b)};
	const int kdn {get_dn_({1, 6})};
	while (true) {//поиск t3 = датa недели по воздвижении пред. года
		int q{ get_dn_prev_year(t3) };
		if(q==0) { break; }
		t3 = increment_date_(t3, 1, b1);
	}
	do {//поиск i = календарный номер t3 по пятидесятнице пред. года
		ddd = increment_date_(ddd, 7, b1);
		i++;
	} while(ddd!=t3);
	int sn   {17-i};//кол-во седмиц осенней отступки/преступки  пред. года
	int osen {17 - get_n50_(ned_po_vozdv)};//тоже для текущего года
	int zimn {};//расчет кол-во седмиц зимней отступки (А.Кашкин - стр.126)...
	if( !(dd==d2 && kdn!=0 && kdn!=1) ) {
		if( dd==d2 && (kdn==0||kdn==1) ) zimn--;
		if( dd!=d2 ) {
			if(kdn==0 || kdn==1) zimn--;
			auto d3 {d2};
			while(d3!=dd) {
				d3 = increment_date_(d3, 7, b);
				zimn--;
			}
		}
	}
	if(zimn!=0) {//поиск ddd = дата начала нового ряда чтений
		switch(kdn) {
		case 1: {  }
		case 0: { ddd = {1, 7}; } break;
		default:{ ddd = increment_date_(d2, 1, b); } };
	} else {
		ddd = {-1,-1};
	}
	switch(zimn) {//выбор номеров добавочных седмиц из опций класса
		case -1: { v.push_back(zimn_otstupka_n1); } break;
		case -2: {
			v.resize(zimn_otstupka_n2.size());
			std::reverse_copy(zimn_otstupka_n2.begin(), zimn_otstupka_n2.end(), std::begin(v));
		} break;
		case -3: {
			v.resize(zimn_otstupka_n3.size());
			std::reverse_copy(zimn_otstupka_n3.begin(), zimn_otstupka_n3.end(), std::begin(v));
		} break;
		case -4: {
			v.resize(zimn_otstupka_n4.size());
			std::reverse_copy(zimn_otstupka_n4.begin(), zimn_otstupka_n4.end(), std::begin(v));
		} break;
		case -5: {
			v.resize(zimn_otstupka_n5.size());
			std::reverse_copy(zimn_otstupka_n5.begin(), zimn_otstupka_n5.end(), std::begin(v));
		} break;
		default: {}
	};
	switch(std::abs(zimn)-1) {//выбор номеров добавочных недель
		case 4: {
			w.push_back(32); w.push_back(17); w.push_back(31); w.push_back(30);
		} break;
		case 3: {
			w.push_back(32); w.push_back(31); w.push_back(30);
		} break;
		case 2: {
			w.push_back(32); w.push_back(31);
		} break;
		case 1: {
			w.push_back(32);
		} break;
		default: {}
	};
	v1 = v; w1 = w;//копия для вычислений апостола
	winter_indent = zimn; spring_indent = osen; //сохранение в объекте
	t3 = get_date_(ned8_popashe);//пятидесятницa
	while(true) {//цикл перебора дат всего года
		j = get_dn_(t1);
		//период от начала года до субб.перед нед.омытариифарисеи вкл. без отступки
		//+период от начала года до начала нового ряда чтений при наличии отступки
		if( (zimn!=0 && t1<ddd) || (zimn==0 && t1<dd) ) {
			int k { sn==0 ? sn : (sn>0 ? -sn : std::abs(sn)) };
			set_evangelie_(t1, evangelie_table1_get_chteniya(get_n50_(t1)-k, j));
		}
		//период нового ряда чтений при наличии отступки (до субб.перед нед.омытариифарисеи вкл.)
		if(zimn!=0 && t1>=ddd && t1<dd && j==0) {
			if(!w.empty()) {
				set_evangelie_(t1, evangelie_table1_get_chteniya(w.back(), j));
				w.pop_back();
			}
			if(!v.empty()) v.pop_back();
		}
		if(zimn!=0 && t1>=ddd && t1<dd && j!=0 && !v.empty()) {
			set_evangelie_(t1, evangelie_table1_get_chteniya(v.back(), j));
		}
		//период от нед. о мытари и фарисеи до прощ. воскр. вкл.
		if(t1==dd) {
			set_evangelie_(t1, evangelie_table1_get_chteniya(33, j));
		}
		if(t1>dd && t1<=mf7) {
			set_evangelie_(t1, evangelie_table1_get_chteniya(34, j));
		}
		if(t1>mf7 && t1<=mf14) {
			set_evangelie_(t1, evangelie_table1_get_chteniya(35, j));
		}
		if(t1>mf14 && t1<=mf21) {
			set_evangelie_(t1, evangelie_table1_get_chteniya(36, j));
		}
		//период от начала в.поста до троицкой род.субб вкл.
		if(t1>mf21 && t1<t3) {
			auto fr1 = days.find(t1);
			if(fr1 != days.end()) {
				set_evangelie_(t1, evangelie_table2_get_chteniya(fr1->second.day_markers));
			}
		}
		//период от пятидесятницы до конца года
		if( (t1>=t3 && t1<=dd1) || (t1>dd1 && t1<=ned_po_vozdv && osen>=0) ) {
			set_evangelie_(t1, evangelie_table1_get_chteniya(get_n50_(t1), j));
		}
		if(t1>dd1 && t1<=dd2 && osen<0) {
			if(osen==-2)
				set_evangelie_(t1, evangelie_table1_get_chteniya(osen_otstupka.front(), j));
			else
				set_evangelie_(t1, evangelie_table1_get_chteniya(get_n50_(t1), j));
		}
		if(t1>dd2 && t1<=ned_po_vozdv && osen<0) {
			set_evangelie_(t1, evangelie_table1_get_chteniya(osen_otstupka.back(), j));
		}
		if(t1>ned_po_vozdv) {
			int k { osen==0 ? osen : (osen>0 ? -osen : std::abs(osen)) };
			set_evangelie_(t1, evangelie_table1_get_chteniya(get_n50_(t1)-k, j));
		}
		//конец цикла
		t2 = increment_date_(t1, 1, b);
		if(t2!=t1) { t1 = t2; }
		else       { break; }
	}
	//расчет рядовые чтения апостола на литургии
	t1 = make_pair(1,1);
	while(true) {//цикл перебора дат всего года
		j = get_dn_(t1);
		//период от начала года до субб.перед нед.омытариифарисеи вкл. без отступки
		//+период от начала года до начала нового ряда чтений при наличии отступки
		if( (zimn!=0 && t1<ddd) || (zimn==0 && t1<dd) ) {
			set_apostol_(t1, apostol_table1_get_chteniya(get_n50_(t1), j));
		}
		//период нового ряда чтений при наличии отступки (до субб.перед нед.омытариифарисеи вкл.)
		if(zimn!=0 && t1>=ddd && t1<dd && j==0) {
			if(!w1.empty()) {
				set_apostol_(t1, apostol_table1_get_chteniya(w1.back(), j));
				w1.pop_back();
			}
			if(!v1.empty()) v1.pop_back();
		}
		if(zimn!=0 && t1>=ddd && t1<dd && j!=0 && !v1.empty()) {
			set_apostol_(t1, apostol_table1_get_chteniya(v1.back(), j));
		}
		//период от нед. о мытари и фарисеи до прощ. воскр. вкл.
		if(t1==dd) {
			set_apostol_(t1, apostol_table1_get_chteniya(33, j));
		}
		if(t1>dd && t1<=mf7) {
			set_apostol_(t1, apostol_table1_get_chteniya(34, j));
		}
		if(t1>mf7 && t1<=mf14) {
			set_apostol_(t1, apostol_table1_get_chteniya(35, j));
		}
		if(t1>mf14 && t1<=mf21) {
			set_apostol_(t1, apostol_table1_get_chteniya(36, j));
		}
		//период от начала в.поста до троицкой род.субб вкл.
		if(t1>mf21 && t1<t3) {
			auto fr1 = days.find(t1);
			if(fr1 != days.end()) {
				set_apostol_(t1, apostol_table2_get_chteniya(fr1->second.day_markers));
			}
		}
		//период от пятидесятницы до конца года
		if(t1>=t3) {
			if(!osen_otstupka_apostol) {
				set_apostol_(t1, apostol_table1_get_chteniya(get_n50_(t1), j));
			} else {
				if( (t1>=t3 && t1<=dd1) || (t1>dd1 && t1<=ned_po_vozdv && osen>=0) ) {
					set_apostol_(t1, apostol_table1_get_chteniya(get_n50_(t1), j));
				}
				if(t1>dd1 && t1<=dd2 && osen<0) {
					if(osen==-2)
						set_apostol_(t1, apostol_table1_get_chteniya(osen_otstupka.front(), j));
					else
						set_apostol_(t1, apostol_table1_get_chteniya(get_n50_(t1), j));
				}
				if(t1>dd2 && t1<=ned_po_vozdv && osen<0) {
					set_apostol_(t1, apostol_table1_get_chteniya(osen_otstupka.back(), j));
				}
				if(t1>ned_po_vozdv) {
					int k { osen==0 ? osen : (osen>0 ? -osen : std::abs(osen)) };
					set_apostol_(t1, apostol_table1_get_chteniya(get_n50_(t1)-k, j));
				}
			}
		}
		t2 = increment_date_(t1, 1, b);
		if(t2!=t1) { t1 = t2; }
		else       { break; }
	}
	//save data to object
	data1.reserve(days.size());
	std::for_each(days.begin(), days.end(), [this](const auto& e){
		Data1 d;
		d.dn = e.second.dn;
		d.glas = e.second.glas;
		d.n50 = e.second.n50;
		d.day = e.first.second;
		d.month = e.first.first;
		d.apostol = e.second.apostol;
		d.evangelie = e.second.evangelie;
		std::copy(e.second.day_markers.begin(), e.second.day_markers.end(), d.day_markers.begin());
		data1.push_back(std::move(d));
	});
	data2.reserve(markers.size());
	std::for_each(markers.begin(), markers.end(), [this](const auto& e){
		Data2 d;
		d.marker = e.first;
		d.day = e.second.second;
		d.month = e.second.first;
		data2.push_back(std::move(d));
	});
	data1.shrink_to_fit();
	data2.shrink_to_fit();
}//end OrthYear ctor

std::optional<decltype(OrthYear::data1)::const_iterator> OrthYear::find_in_data1(int8_t m, int8_t d) const
{
	auto dd = ShortDate{m, d};
	auto fr = std::lower_bound(data1.begin(), data1.end(), dd);
	if(fr==data1.end()) return std::nullopt;
	if( !(*fr==dd) ) return std::nullopt;
	return fr;
}

int8_t OrthYear::get_winter_indent() const
{
	return winter_indent;
}

int8_t OrthYear::get_spring_indent() const
{
	return spring_indent;
}

int8_t OrthYear::get_date_glas(int8_t month, int8_t day) const
{
	if(auto fr = find_in_data1(month, day); fr) {
		return fr.value()->glas;
	} else {
		return -1;
	}
}

int8_t OrthYear::get_date_n50(int8_t month, int8_t day) const
{
	if(auto fr = find_in_data1(month, day); fr) {
		return fr.value()->n50;
	} else {
		return -1;
	}
}

int8_t OrthYear::get_date_dn(int8_t month, int8_t day) const
{
	if(auto fr = find_in_data1(month, day); fr) {
		return fr.value()->dn;
	} else {
		return -1;
	}
}

ApEvReads OrthYear::get_date_apostol(int8_t month, int8_t day) const
{
	if(auto fr = find_in_data1(month, day); fr) {
		return fr.value()->apostol;
	} else {
		return {};
	}
}

ApEvReads OrthYear::get_date_evangelie(int8_t month, int8_t day) const
{
	if(auto fr = find_in_data1(month, day); fr) {
		return fr.value()->evangelie;
	} else {
		return {};
	}
}

ApEvReads OrthYear::get_resurrect_evangelie(int8_t month, int8_t day) const
{
	auto dn = get_date_dn(month, day);
	if(dn != 0) return {};
	//таблица 11-и воскресныx утрених евангелий
	static const std::array resurrect_evangelie_table = {
		ApEvReads{ 116, "Мф., 116 зач., XXVIII, 16–20." },
		ApEvReads{ 70 , "Мк., 70 зач., XVI, 1–8." },
		ApEvReads{ 71 , "Мк., 71 зач., XVI, 9–20." },
		ApEvReads{ 112, "Лк., 112 зач., XXIV, 1–12." },
		ApEvReads{ 113, "Лк., 113 зач., XXIV, 12–35." },
		ApEvReads{ 114, "Лк., 114 зач., XXIV, 36–53." },
		ApEvReads{ 63 , "Ин., 63 зач., XX, 1–10." },
		ApEvReads{ 64 , "Ин., 64 зач., XX, 11–18." },
		ApEvReads{ 65 , "Ин., 65 зач., XX, 19–31." },
		ApEvReads{ 66 , "Ин., 66 зач., XXI, 1–14." },
		ApEvReads{ 67 , "Ин., 67 зач., XXI, 15–25." }
	};
	//таблица праздничных утрених евангелий
	static const std::array holydays_evangelie_table = {
		ApEvReads{  83, "Мф., 83 зач., XXI, 1–11, 15–17." },//Вербное воскресенье
		ApEvReads{   2, "Мк., 2 зач., I, 9–11." },          //Крещение
		ApEvReads{   8, "Лк., 8 зач., II, 25–32."},         //Сре́тение
		ApEvReads{   4, "Лк., 4 зач., I, 39–49, 56."},      //Благовещ́ение, Успе́ние, Рождество, Введе́ние Пресв.Богородицы
		ApEvReads{  45, "Лк., 45 зач., IX, 28–36."},        //Преображение
		ApEvReads{  42, "Ин., 42 зач., XII, 28-36."},       //Воздви́жение
		ApEvReads{   2, "Мф., 2 зач., I, 18–25."}           //Рождество
	};
	static const std::array unique_evangelie_table = {
		ned2_popashe,
		ned3_popashe,
		ned4_popashe,
		ned5_popashe,
		ned6_popashe,
		ned7_popashe,
		ned8_popashe,
		vel_post_d0n7,
		m1d6,
		sretenie,
		m3d25,
		m8d6,
		m8d15,
		m9d8,
		m9d14,
		m11d21,
		m12d25
	};
	auto w = unique_evangelie_table.end();
	if( auto date_properties = get_date_properties(month, day); date_properties ) {
		w = std::find_first_of(unique_evangelie_table.begin(),
															unique_evangelie_table.end(),
															date_properties->begin(),
															date_properties->end());
	}
	if( w != unique_evangelie_table.end() ) {
		switch(*w) {
			case ned2_popashe:  { return resurrect_evangelie_table[0]; }
			case ned3_popashe:  { return resurrect_evangelie_table[2]; }
			case ned4_popashe:  { return resurrect_evangelie_table[3]; }
			case ned5_popashe:  { return resurrect_evangelie_table[6]; }
			case ned6_popashe:  { return resurrect_evangelie_table[7]; }
			case ned7_popashe:  { return resurrect_evangelie_table[9]; }
			case ned8_popashe:  { return resurrect_evangelie_table[8]; }
			case vel_post_d0n7: { return holydays_evangelie_table[0]; }
			case m1d6:          { return holydays_evangelie_table[1]; }
			case sretenie:      { return holydays_evangelie_table[2]; }
			case m3d25:         { return holydays_evangelie_table[3]; }
			case m8d6:          { return holydays_evangelie_table[4]; }
			case m8d15:         { return holydays_evangelie_table[3]; }
			case m9d8:          { return holydays_evangelie_table[3]; }
			case m9d14:         { return holydays_evangelie_table[5]; }
			case m11d21:        { return holydays_evangelie_table[3]; }
			case m12d25:        { return holydays_evangelie_table[6]; }
			default:            { return {}; }
		};
	} else {
		auto n50 = get_date_n50(month, day);
		if(n50>0 && n50<12) {
			return resurrect_evangelie_table[n50-1] ;
		} else if(n50>11) {
			uint8_t x = n50 % 11;
			if(x==0) x = 10; else x--;
			return resurrect_evangelie_table[x] ;
		}
	}
	return {};
}

std::optional<std::vector<uint16_t>> OrthYear::get_date_properties(int8_t month, int8_t day) const
{
	if(auto fr = find_in_data1(month, day); fr) {
		std::vector<uint16_t> res ;
		std::copy_if(fr.value()->day_markers.begin(), fr.value()->day_markers.end(),
									std::back_inserter(res),
									[](auto x){ return x>0; });
		if(res.empty()) return std::nullopt;
		return res;
	} else {
		return std::nullopt;
	}
}

std::optional<ShortDate> OrthYear::get_date_with(uint16_t m) const
{
	if(m<1) return std::nullopt;
	auto d = static_cast<uint16_t>(m);
	auto fr = std::lower_bound(data2.begin(), data2.end(), d);
	if(fr==data2.end()) return std::nullopt;
	if( !(*fr==d) ) return std::nullopt;
	return ShortDate{fr->month, fr->day};
}

std::optional<std::vector<ShortDate>> OrthYear::get_alldates_with(uint16_t m) const
{
	if(m<1) return std::nullopt;
	auto d = static_cast<uint16_t>(m);
	auto [begin, end] = std::equal_range(data2.begin(), data2.end(), d);
	if(begin==data2.end()) return std::nullopt;
	int sz = std::distance(begin, end);
	if(sz<1) return std::nullopt;
	std::vector<ShortDate> res (static_cast<size_t>(sz));
	std::transform(begin, end, res.begin(), [](const auto& e){ return ShortDate{e.month, e.day}; });
	return res;
}

std::optional<ShortDate> OrthYear::get_date_withanyof(std::span<uint16_t> m) const
{
	if(m.empty()) return std::nullopt;
	for(auto i: m) { if(auto x = get_date_with(i); x) return *x; }
	return std::nullopt;
}

std::optional<ShortDate> OrthYear::get_date_withallof(std::span<uint16_t> m) const
{
	if(m.empty()) return std::nullopt;
	auto semires = get_alldates_with(m.front());
	if(!semires) return std::nullopt;
	std::vector<uint16_t> v (m.size());
	std::transform(m.begin(), m.end(), v.begin(), [](auto x){ return static_cast<uint16_t>(x); });
	std::sort(v.begin(), v.end());
	for(auto [month, day] : *semires) {
		if(auto fr = find_in_data1(month, day); fr) {
			auto begin = fr.value()->day_markers.begin();
			auto end = fr.value()->day_markers.end();
			auto sr = std::search(begin, end, v.begin(), v.end());
			if(sr != end) return ShortDate{month, day};
		}
	}
	return std::nullopt;
}

std::optional<std::vector<ShortDate>> OrthYear::get_alldates_withanyof(std::span<uint16_t> m) const
{
	if(m.empty()) return std::nullopt;
	std::vector<ShortDate> result;
	for(auto i: m) {
		if(auto x=get_alldates_with(i); x)
			std::copy(x->begin(), x->end(), std::back_inserter(result));
	}
	if(result.empty()) return std::nullopt;
	else return result;
}

std::string OrthYear::get_description_forday(int8_t month, int8_t day) const
{
	//таблица - названия перeходящих дней
	static const std::map<uint16_t, std::string_view> nostable_dates_str = {
		{pasha,              "Светлое Христово Воскресение. ПАСХА."},
		{svetlaya1,          "Понедельник Светлой седмицы."},
		{svetlaya2,          "Вторник Светлой седмицы. Иверской иконы Божией Матери."},
		{svetlaya3,          "Среда Светлой седмицы."},
		{svetlaya4,          "Четверг Светлой седмицы."},
		{svetlaya5,          "Пятница Светлой седмицы. Последование в честь Пресвятой Богородицы ради Ее «Живоно́сного Исто́чника»."},
		{svetlaya6,          "Суббота Светлой седмицы."},
		{ned2_popashe,       "Неделя 2-я по Пасхе, апостола Фомы́ . Антипасха."},
		{s2popashe_1,        "Понедельник 2-й седмицы по Пасхе."},
		{s2popashe_2,        "Вторник 2-й седмицы по Пасхе. Ра́доница. Поминовение усопших."},
		{s2popashe_3,        "Среда 2-й седмицы по Пасхе."},
		{s2popashe_4,        "Четверг 2-й седмицы по Пасхе."},
		{s2popashe_5,        "Пятница 2-й седмицы по Пасхе."},
		{s2popashe_6,        "Суббота 2-й седмицы по Пасхе."},
		{ned3_popashe,       "Неделя 3-я по Пасхе, святых жен-мироносиц. Правв. Марфы и Марии, сестер прав. Лазаря."},
		{s3popashe_1,        "Понедельник 3-й седмицы по Пасхе."},
		{s3popashe_2,        "Вторник 3-й седмицы по Пасхе."},
		{s3popashe_3,        "Среда 3-й седмицы по Пасхе."},
		{s3popashe_4,        "Четверг 3-й седмицы по Пасхе."},
		{s3popashe_5,        "Пятница 3-й седмицы по Пасхе."},
		{s3popashe_6,        "Суббота 3-й седмицы по Пасхе."},
		{ned4_popashe,       "Неделя 4-я по Пасхе, о расслабленном."},
		{s4popashe_1,        "Понедельник 4-й седмицы по Пасхе."},
		{s4popashe_2,        "Вторник 4-й седмицы по Пасхе."},
		{s4popashe_3,        "Среда 4-й седмицы по Пасхе. Преполове́ние Пятидесятницы."},
		{s4popashe_4,        "Четверг 4-й седмицы по Пасхе."},
		{s4popashe_5,        "Пятница 4-й седмицы по Пасхе."},
		{s4popashe_6,        "Суббота 4-й седмицы по Пасхе."},
		{ned5_popashe,       "Неделя 5-я по Пасхе, о самаряны́не."},
		{s5popashe_1,        "Понедельник 5-й седмицы по Пасхе."},
		{s5popashe_2,        "Вторник 5-й седмицы по Пасхе."},
		{s5popashe_3,        "Среда 5-й седмицы по Пасхе. Отдание праздника Преполовения Пятидесятницы."},
		{s5popashe_4,        "Четверг 5-й седмицы по Пасхе."},
		{s5popashe_5,        "Пятница 5-й седмицы по Пасхе."},
		{s5popashe_6,        "Суббота 5-й седмицы по Пасхе."},
		{ned6_popashe,       "Неделя 6-я по Пасхе, о слепом."},
		{s6popashe_1,        "Понедельник 6-й седмицы по Пасхе."},
		{s6popashe_2,        "Вторник 6-й седмицы по Пасхе."},
		{s6popashe_3,        "Среда 6-й седмицы по Пасхе. Отдание праздника Пасхи. Предпразднство Вознесения."},
		{s6popashe_4,        "Четверг 6-й седмицы по Пасхе. Вознесе́ние Госпо́дне."},
		{s6popashe_5,        "Пятница 6-й седмицы по Пасхе. Попразднство Вознесения."},
		{s6popashe_6,        "Суббота 6-й седмицы по Пасхе. Попразднство Вознесения."},
		{ned7_popashe,       "Неделя 7-я по Пасхе, святых 318 богоносных отцов Первого Вселенского Собора. Попразднство Вознесения."},
		{s7popashe_1,        "Понедельник 7-й седмицы по Пасхе. Попразднство Вознесения."},
		{s7popashe_2,        "Вторник 7-й седмицы по Пасхе. Попразднство Вознесения."},
		{s7popashe_3,        "Среда 7-й седмицы по Пасхе. Попразднство Вознесения."},
		{s7popashe_4,        "Четверг 7-й седмицы по Пасхе. Попразднство Вознесения."},
		{s7popashe_5,        "Пятница 7-й седмицы по Пасхе. Отдание праздника Вознесения Господня."},
		{s7popashe_6,        "Суббота 7-й седмицы по Пасхе. Троицкая родительская суббота."},
		{ned8_popashe,       "Неделя 8-я по Пасхе. День Святой Тро́ицы. Пятидеся́тница."},
		{s1po50_1,           "Понедельник Пятидесятницы. День Святаго Духа."},
		{s1po50_2,           "Вторник Пятидесятницы."},
		{s1po50_3,           "Среда Пятидесятницы."},
		{s1po50_4,           "Четверг Пятидесятницы."},
		{s1po50_5,           "Пятница Пятидесятницы."},
		{s1po50_6,           "Суббота Пятидесятницы. Отдание праздника Пятидесятницы."},
		{ned1_po50,          "Неделя 1-я по Пятидесятнице, Всех святых."},
		{pyatnica9_popashe,  "Прп. Варлаа́ма Ху́тынского. Табы́нской иконы Божией Матери."},
		{ned2_po50,          "Неделя 2-я по Пятидесятнице, Всех святых, в земле Русской просиявших."},
		{ned3_po50,          "Неделя 3-я по Пятидесятнице. Собор всех новоявле́нных мучеников Христовых по взятии Царяграда пострадавших. Собор Новгородских святых. Собор Белорусских святых. Собор святых Санкт-Петербургской митрополии."},
		{ned4_po50,          "Неделя 4-я по Пятидесятнице. Собор преподобных отцов Псково-Печерских."},
		{ned_popreobrajenii, "Собо́р преподо́бных отце́в, на Валаа́ме просия́вших."},
		{ned_pered6sent,     "Перенесение мощей блгвв. кн. Петра, в иночестве Давида, и кн. Февронии, в иночестве Евфросинии, Муромских чудотворцев."},
		{sub_pered14sent,    "Суббота пред Воздвижением."},
		{ned_pered14sent,    "Неделя пред Воздвижением."},
		{sub_po14sent,       "Суббота по Воздвижении."},
		{ned_po14sent,       "Неделя по Воздвижении."},
		{ned_pod11okt,       "Память святых отцов VII Вселенского Собора."},
		{sub_dmitry,         "Димитриевская родительская суббота."},
		{ned_pod1noyabr,     "Собор всех Бессребреников."},
		{ned_praotec,        "Неделя святых пра́отец."},
		{sub_peredrojd,      "Суббота пред Рождеством Христовым."},
		{ned_peredrojd,      "Неделя пред Рождеством Христовым, святых отец."},
		{sub_porojdestve,    "Чтения субботы по Рождестве Христовом."},
		{ned_porojdestve,    "Чтения недели по Рождестве Христовом. Праведных Ио́сифа Обру́чника, Дави́да царя и Иа́кова, брата Господня."},
		{ned_mitar_ifaris,   "Неделя о мытаре́ и фарисе́е."},
		{ned_obludnom,       "Неделя о блудном сыне."},
		{sub_myasopust,      "Суббота мясопу́стная. Вселенская родительская суббота."},
		{ned_myasopust,      "Неделя мясопу́стная, о Страшном Суде."},
		{sirnaya1,           "Понедельник сырный."},
		{sirnaya2,           "Вторник сырный."},
		{sirnaya3,           "Среда сырная."},
		{sirnaya4,           "Четверг сырный."},
		{sirnaya5,           "Пятница сырная."},
		{sirnaya6,           "Суббота сырная. Всех преподобных отцов, в подвиге просиявших."},
		{ned_siropust,       "Неделя сыропустная. Воспоминание Адамова изгнания. Прощеное воскресенье."},
		{vel_post_d1n1,      "Понедельник 1-й седмицы. Начало Великого поста."},
		{vel_post_d2n1,      "Вторник 1-й седмицы великого поста."},
		{vel_post_d3n1,      "Среда 1-й седмицы великого поста."},
		{vel_post_d4n1,      "Четверг 1-й седмицы великого поста."},
		{vel_post_d5n1,      "Пятница 1-й седмицы великого поста."},
		{vel_post_d6n1,      "Суббота 1-й седмицы великого поста."},
		{vel_post_d0n2,      "Неделя 1-я Великого поста. Торжество Православия."},
		{vel_post_d1n2,      "Понедельник 2-й седмицы великого поста."},
		{vel_post_d2n2,      "Вторник 2-й седмицы великого поста."},
		{vel_post_d3n2,      "Среда 2-й седмицы великого поста."},
		{vel_post_d4n2,      "Четверг 2-й седмицы великого поста."},
		{vel_post_d5n2,      "Пятница 2-й седмицы великого поста."},
		{vel_post_d6n2,      "Суббота 2-й седмицы великого поста."},
		{vel_post_d0n3,      "Неделя 2-я Великого поста. Свт. Григория Пала́мы, архиеп. Фессалони́тского. Собор преподобных отец Киево-Печерских и всех святых, в Малой России просиявших."},
		{vel_post_d1n3,      "Понедельник 3-й седмицы великого поста."},
		{vel_post_d2n3,      "Вторник 3-й седмицы великого поста."},
		{vel_post_d3n3,      "Среда 3-й седмицы великого поста."},
		{vel_post_d4n3,      "Четверг 3-й седмицы великого поста."},
		{vel_post_d5n3,      "Пятница 3-й седмицы великого поста."},
		{vel_post_d6n3,      "Суббота 3-й седмицы великого поста."},
		{vel_post_d0n4,      "Неделя 3-я Великого поста, Крестопоклонная."},
		{vel_post_d1n4,      "Понедельник 4-й седмицы вел. поста, Крестопоклонной."},
		{vel_post_d2n4,      "Вторник 4-й седмицы вел. поста, Крестопоклонной."},
		{vel_post_d3n4,      "Среда 4-й седмицы вел. поста, Крестопоклонной."},
		{vel_post_d4n4,      "Четверг 4-й седмицы вел. поста, Крестопоклонной."},
		{vel_post_d5n4,      "Пятница 4-й седмицы вел. поста, Крестопоклонной."},
		{vel_post_d6n4,      "Суббота 4-й седмицы вел. поста, Крестопоклонной."},
		{vel_post_d0n5,      "Неделя 4-я Великого поста. Прп. Иоанна Ле́ствичника."},
		{vel_post_d1n5,      "Понедельник 5-й седмицы великого поста."},
		{vel_post_d2n5,      "Вторник 5-й седмицы великого поста."},
		{vel_post_d3n5,      "Среда 5-й седмицы великого поста."},
		{vel_post_d4n5,      "Четверг 5-й седмицы великого поста."},
		{vel_post_d5n5,      "Пятница 5-й седмицы великого поста."},
		{vel_post_d6n5,      "Суббота 5-й седмицы великого поста. Суббота Ака́фиста. Похвала́ Пресвятой Богородицы."},
		{vel_post_d0n6,      "Неделя 5-я Великого поста. Прп. Марии Египетской."},
		{vel_post_d1n6,      "Понедельник 6-й седмицы великого поста. ва́ий."},
		{vel_post_d2n6,      "Вторник 6-й седмицы великого поста. ва́ий."},
		{vel_post_d3n6,      "Среда 6-й седмицы великого поста. ва́ий."},
		{vel_post_d4n6,      "Четверг 6-й седмицы великого поста. ва́ий."},
		{vel_post_d5n6,      "Пятница 6-й седмицы великого поста. ва́ий."},
		{vel_post_d6n6,      "Суббота 6-й седмицы великого поста. ва́ий. Лазарева суббота. Воскрешение прав. Лазаря."},
		{vel_post_d0n7,      "Неделя ва́ий (цветоно́сная, Вербное воскресенье). Вход Господень в Иерусалим."},
		{vel_post_d1n7,      "Страстна́я седмица. Великий Понедельник."},
		{vel_post_d2n7,      "Страстна́я седмица. Великий Вторник."},
		{vel_post_d3n7,      "Страстна́я седмица. Великая Среда."},
		{vel_post_d4n7,      "Страстна́я седмица. Великий Четверг. Воспоминание Тайной Ве́чери."},
		{vel_post_d5n7,      "Страстна́я седмица. Великая Пятница."},
		{vel_post_d6n7,      "Страстна́я седмица. Великая Суббота."}
	};
	//таблица - названия неперeходящих праздничных дней
	static const std::map<uint16_t, std::string_view> stable_dates_str = {
		{m1d1,  "Обре́зание Господне. Свт. Василия Великого, архиеп. Кесари́и Каппадоки́йской."},
		{m1d2,  "Предпразднство Богоявления. Прп. Серафи́ма Саро́вского."},
		{m1d3,  "Предпразднство Богоявления. Прор. Малахи́и. Мч. Горди́я."},
		{m1d4,  "Предпразднство Богоявления. Собор 70-ти апостолов. Прп. Феокти́ста, игумена Куку́ма Сикели́йского."},
		{m1d5,  "Предпразднство Богоявления. На́вечерие Богоявления (Крещенский сочельник). Сщмч. Феопе́мпта, еп. Никомиди́йского, и мч. Фео́ны волхва. Прп. Синклитики́и Александрийской. День постный."},
		{m1d6,  "Святое Богоявле́ние. Крещение Господа Бога и Спаса нашего Иисуса Христа."},
		{m1d7,  "Попразднство Богоявления. Собор Предтечи и Крестителя Господня Иоанна."},
		{m1d8,  "Попразднство Богоявления. Прп. Гео́ргия Хозеви́та. Прп. Домни́ки."},
		{m1d9,  "Попразднство Богоявления. Мч. Полие́вкта. Свт. Фили́ппа, митр. Московского и всея России, чудотворца."},
		{m1d10, "Попразднство Богоявления. Свт. Григория, еп. Ни́сского. Прп. Дометиа́на, еп. Мелити́нского. Свт. Феофа́на, Затворника Вы́шенского."},
		{m1d11, "Попразднство Богоявления. Прп. Феодо́сия Великого, общих жити́й начальника."},
		{m1d12, "Попразднство Богоявления. Мц. Татиа́ны."},
		{m1d13, "Попразднство Богоявления. Мчч. Ерми́ла и Стратони́ка. Прп. Ирина́рха, затворника Ростовского."},
		{m1d14, "Отдание праздника Богоявления. Св. равноап. Нины, просветительницы Грузии."},
		{m3d25, "Благовещ́ение Пресвято́й Богоро́дицы."},
		{m5d11, "Равноапп. Мефо́дия и Кири́лла, учи́телей Слове́нских."},
		{m6d24, "Рождество́ честно́го сла́вного Проро́ка, Предте́чи и Крести́теля Госпо́дня Иоа́нна."},
		{m6d25, "Отдание праздника рождества Предте́чи и Крести́теля Госпо́дня Иоа́нна. Прмц. Февро́нии."},
		{m6d29, "Славных и всехва́льных первоверхо́вных апостолов Петра и Павла."},
		{m6d30, "Собор славных и всехвальных 12-ти апостолов."},
		{m7d15, "Равноап. вел. князя Влади́мира, во Святом Крещении Васи́лия."},
		{m8d5,  "Предпразднство Преображения Господня. Мч. Евсигни́я."},
		{m8d6,  "Преображение Господа Бога и Спаса нашего Иисуса Христа."},
		{m8d7,  "Попразднство Преображения Господня. Прмч. Домети́я. Обре́тение моще́й свт. Митрофа́на, еп. Воро́нежского."},
		{m8d8,  "Попразднство Преображения Господня. Свт. Емилиа́на исп., еп. Кизи́ческого. Перенесение мощей прпп. Зоси́мы, Савва́тия и Ге́рмана Солове́цких."},
		{m8d9,  "Попразднство Преображения Господня. Апостола Матфи́я."},
		{m8d10, "Попразднство Преображения Господня. Мч. архидиакона Лавре́нтия. Собор новомучеников и исповедников Солове́цких."},
		{m8d11, "Попразднство Преображения Господня. Мч. архидиакона Е́впла."},
		{m8d12, "Попразднство Преображения Господня. Мчч. Фо́тия и Аники́ты. Прп. Макси́ма Испове́дника."},
		{m8d13, "Отдание праздника Преображения Господня. Свт. Ти́хона, еп. Воро́нежского, Задо́нского, чудотворца."},
		{m8d14, "Предпразднство Успения Пресвятой Богородицы. Прор. Михе́я. Перенесение мощей прп. Феодо́сия Пече́рского."},
		{m8d15, "Успе́ние Пресвятой Владычицы нашей Богородицы и Приснодевы Марии."},
		{m8d16, "Попразднство Успения Пресвятой Богородицы. Перенесение из Еде́ссы в Константино́поль Нерукотворе́нного О́браза (Убру́са) Господа Иисуса Христа."},
		{m8d17, "Попразднство Успения Пресвятой Богородицы. Мч. Ми́рона."},
		{m8d18, "Попразднство Успения Пресвятой Богородицы. Мчч. Фло́ра и Ла́вра."},
		{m8d19, "Попразднство Успения Пресвятой Богородицы. Мч. Андрея Стратила́та и иже с ним. Донской иконы Божией Матери."},
		{m8d20, "Попразднство Успения Пресвятой Богородицы. Прор. Самуила."},
		{m8d21, "Попразднство Успения Пресвятой Богородицы. Ап. от 70-ти Фадде́я. Мц. Ва́ссы."},
		{m8d22, "Попразднство Успения Пресвятой Богородицы. Мч. Агафони́ка и иже с ним. Мч. Лу́ппа."},
		{m8d23, "Отдание праздника Успения Пресвятой Богородицы."},
		{m9d7,  "Предпразднство Рождества Пресвятой Богородицы. Мч. Созонта."},
		{m9d8,  "Рождество Пресвятой Владычицы нашей Богородицы и Приснодевы Марии."},
		{m9d9,  "Попразднство Рождества Пресвятой Богородицы. Праведных Богооте́ц Иоаки́ма и А́нны. Прп. Ио́сифа, игумена Во́лоцкого, чудотворца."},
		{m9d10, "Попразднство Рождества Пресвятой Богородицы. Мцц. Минодо́ры, Митродо́ры и Нимфодо́ры."},
		{m9d11, "Попразднство Рождества Пресвятой Богородицы. Прп. Силуа́на Афо́нского."},
		{m9d12, "Отдание праздника Рождества Пресвятой Богородицы."},
		{m9d13, "Предпразднство Воздви́жения Честно́го и Животворя́щего Креста Господня. Сщмч. Корни́лия со́тника."},
		{m9d14, "Всеми́рное Воздви́жение Честно́го и Животворя́щего Креста́ Госпо́дня. День постный."},
		{m9d15, "Попразднство Воздвижения Креста. Вмч. Ники́ты."},
		{m9d16, "Попразднство Воздвижения Креста. Вмц. Евфи́мии всехва́льной."},
		{m9d17, "Попразднство Воздвижения Креста. Мцц. Ве́ры, Наде́жды, Любо́ви и матери их Софи́и."},
		{m9d18, "Попразднство Воздвижения Креста. Прп. Евме́ния, еп. Горти́нского."},
		{m9d19, "Попразднство Воздвижения Креста. Мчч. Трофи́ма, Савва́тия и Доримедо́нта."},
		{m9d20, "Попразднство Воздвижения Креста. Вмч. Евста́фия и иже с ним. Мучеников и исповедников Михаи́ла, кн. Черни́говского, и боля́рина его Фео́дора, чудотворцев."},
		{m9d21, "Отдание праздника Воздвижения Животворящего Креста Господня. Обре́тение мощей свт. Дими́трия, митр. Росто́вского."},
		{m8d29, "Усекновение главы́ Пророка, Предтечи и Крестителя Господня Иоанна. День постный."},
		{m10d1, "Покро́в Пресвятой Владычицы нашей Богородицы и Приснодевы Марии. Ап. от 70-ти Ана́нии. Прп. Рома́на Сладкопе́вца."},
		{m11d20,"Предпразднство Введения (Входа) во храм Пресвятой Богородицы. Прп. Григория Декаполи́та. Свт. Про́кла, архиеп. Константинопольского."},
		{m11d21,"Введе́ние (Вход) во храм Пресвятой Владычицы нашей Богородицы и Приснодевы Марии."},
		{m11d22,"Попразднство Введения. Апп. от 70-ти Филимо́на, Архи́ппа и мц. равноап. Апфи́и."},
		{m11d23,"Попразднство Введения. Блгв. вел. кн. Алекса́ндра Не́вского. Свт. Митрофа́на, в схиме Мака́рия, еп. Воро́нежского."},
		{m11d24,"Попразднство Введения. Вмц. Екатерины. Вмч. Мерку́рия."},
		{m11d25,"Отдание праздника Введения (Входа) во храм Пресвятой Богородицы. Сщмчч. Кли́мента, папы Римского, и Петра́, архиеп. Александри́йского."},
		{m12d20,"Предпразднство Рождества Христова. Сщмч. Игна́тия Богоно́сца. Прав. Иоа́нна Кроншта́дтского."},
		{m12d21,"Предпразднство Рождества Христова. Свт. Петра, митр. Киевского, Московского и всея Руси, чудотворца."},
		{m12d22,"Предпразднство Рождества Христова. Вмц. Анастаси́и Узореши́тельницы."},
		{m12d23,"Предпразднство Рождества Христова. Десяти мучеников, иже в Кри́те."},
		{m12d24,"Предпразднство Рождества Христова. На́вечерие Рождества Христова (Рождественский сочельник). Прмц. Евге́нии."},
		{m12d25,"Рождество Господа Бога и Спаса нашего Иисуса Христа."},
		{m12d26,"Попразднство Рождества Христова. Собор Пресвятой Богородицы."},
		{m12d27,"Попразднство Рождества Христова. Ап. первомч. и архидиа́кона Стефа́на. Прп. Фео́дора Начерта́нного, исп."},
		{m12d28,"Попразднство Рождества Христова. Мучеников 20 000 в Никомидии сожженных."},
		{m12d29,"Попразднство Рождества Христова. Мучеников 14 000 младенцев, от Ирода в Вифлееме избиенных."},
		{m12d30,"Попразднство Рождества Христова. Мц. Ани́сии. Прп. Мела́нии Ри́мляныни (с 31декабря). Свт. Мака́рия, митр. Московского."},
		{m12d31,"Отдание праздника Рождества Христова."}
	};
	//таблица - названия других дней (блок 3)
	static const std::map<uint16_t, std::string_view> other_dates_str = {
		{sub_peredbogoyav_m12,    "Чтения субботы перед Богоявлением."},
		{sub_peredbogoyav_m1,     "Чтения субботы перед Богоявлением."},
		{ned_peredbogoyav,        "Чтения недели перед Богоявлением."},
		{sub_pobogoyav,           "Суббота по Богоявлении."},
		{ned_pobogoyav,           "Неделя по Богоявлении."},
		{ned_pod25yanv,           "Собор новомучеников и исповедников Церкви Русской."},
		{sobor_3sv,               "Собор вселенских учителей и святителей Василия Великого, Григория Богослова и Иоанна Златоустого."},
		{sretenie_predpr,         "Предпразднство Сре́тения Господня."},
		{sretenie,                "Сре́тение Господа Бога и Спаса нашего Иисуса Христа."},
		{sretenie_poprazd1,       "День 1-й Попразднства Сретения Господня."},
		{sretenie_poprazd2,       "День 2-й Попразднства Сретения Господня."},
		{sretenie_poprazd3,       "День 3-й Попразднства Сретения Господня."},
		{sretenie_poprazd4,       "День 4-й Попразднства Сретения Господня."},
		{sretenie_poprazd5,       "День 5-й Попразднства Сретения Господня."},
		{sretenie_poprazd6,       "День 6-й Попразднства Сретения Господня."},
		{sretenie_otdanie,        "Отдание праздника Сретения Господня."},
		{obret_gl_ioanna12,       "Первое и второе Обре́тение главы Иоанна Предтечи."},
		{muchenik_40,             "Святых сорока́ мучеников, в Севастийском е́зере мучившихся."},
		{blag_predprazd,          "Предпразднство Благовещения Пресвятой Богородицы."},
		{blag_otdanie,            "Отдание праздника Благовещения Пресвятой Богородицы. Собор Архангела Гаврии́ла."},
		{georgia_pob,             "Вмч. Гео́ргия Победоно́сца. Мц. царицы Александры."},
		{obret_gl_ioanna3,        "Третье обре́тение главы Предтечи и Крестителя Господня Иоанна."},
		{ned_popetraipavla,       "Собор Тверских святых."},
		{ned_pod16iulya,          "Память святых отцов шести Вселенских Соборов."},
		{ned_pered18avg,          "Собор Кемеровских святых."}
	};
	auto get_date_str = [](int m, int d)->std::string {
		std::string s{};
		switch(m) {
			case 1: { s = "января"; }
			break;
			case 2: { s = "февраля"; }
			break;
			case 3: { s = "марта"; }
			break;
			case 4: { s = "апреля"; }
			break;
			case 5: { s = "мая"; }
			break;
			case 6: { s = "июня"; }
			break;
			case 7: { s = "июля"; }
			break;
			case 8: { s = "августа"; }
			break;
			case 9: { s = "сентября"; }
			break;
			case 10:{ s = "октября"; }
			break;
			case 11:{ s = "ноября"; }
			break;
			case 12:{ s = "декабря"; }
			break;
			default: return std::string{};
		};
		return std::string{std::to_string(d)+' '+s+'.'+' '};
	};
	auto get_dn_str = [](int8_t d) -> std::string {
		std::string s{};
		switch(d) {
			case 0: { s = "Воскресенье"; }
			break;
			case 1: { s = "Понедельник"; }
			break;
			case 2: { s = "Вторник"; }
			break;
			case 3: { s = "Среда"; }
			break;
			case 4: { s = "Четверг"; }
			break;
			case 5: { s = "Пятница"; }
			break;
			case 6: { s = "Суббота"; }
			break;
			default: {}
		};
		return std::string{s+'.'+' '};
	};
	auto get_markers_str = [](std::span<const uint16_t> s)->std::string{
		std::string res;
		for(const auto i: s) {
			if(auto fr1 = nostable_dates_str.find(i); fr1!=nostable_dates_str.end()) res += std::string(fr1->second)+ ' ';
			if(auto fr2 = stable_dates_str.find(i); fr2!=stable_dates_str.end()) res += std::string(fr2->second) + ' ';
			if(auto fr3 = other_dates_str.find(i); fr3!=other_dates_str.end()) res += std::string(fr3->second) + ' ';
		}
		return res;
	};
	auto fr = find_in_data1(month, day);
	if(!fr) return {};
	std::string res, gl, po50;
	if(fr.value()->glas > 0) gl = "глас " + std::to_string(fr.value()->glas) + ". ";
	if(fr.value()->n50 > 0) po50 = std::to_string(fr.value()->n50) + " по Пятидесятнице. ";
	res = get_date_str(month, day) + get_dn_str(fr.value()->dn) + po50 + gl + get_markers_str(fr.value()->day_markers);
	return res;
}

/*----------------------------------------------------*/
/*          class OrthodoxCalendar::impl              */
/*----------------------------------------------------*/

class OrthodoxCalendar::impl {
	std::map<int, std::unique_ptr<OrthYear>> cache;
	std::queue<int> cache_elements_queue;
	size_t cache_max_elements;
	//настройка номеров добавочных седмиц зимней отступкu литургийных чтений
	std::array<uint8_t,5> zimn_otstupka_n5; //при отступке в 5 седмиц.
	std::array<uint8_t,4> zimn_otstupka_n4; //при отступке в 4 седмиц.
	std::array<uint8_t,3> zimn_otstupka_n3; //при отступке в 3 седмиц.
	std::array<uint8_t,2> zimn_otstupka_n2; //при отступке в 2 седмиц.
	uint8_t               zimn_otstupka_n1; //при отступке в 1 седмиц.
	//настройка номеров добавочных седмиц осенней отступкu литургийных чтений
	std::array<uint8_t,2> osen_otstupka;
	bool osen_otstupka_apostol; //при вычислении осенней отступкu учитывать ли апостол
	OrthYear* get_from_cache(int& y, int8_t& m, int8_t& d, const bool julian=true);
	OrthYear* get_from_cache(int y);
public:
	explicit impl(size_t sz=3000)
		: cache_max_elements    {sz>0 ? sz : 1},
			zimn_otstupka_n5      {30,31,17,32,33},
			zimn_otstupka_n4      {30,31,32,33},
			zimn_otstupka_n3      {31,32,33},
			zimn_otstupka_n2      {32,33},
			zimn_otstupka_n1      {33},
			osen_otstupka         {10,11},
			osen_otstupka_apostol {false}
	{
	}
	void set_cache_size(size_t sz);
	void set_winter_indent_weeks_1(uint8_t w1);
	void set_winter_indent_weeks_2(uint8_t w1, uint8_t w2);
	void set_winter_indent_weeks_3(uint8_t w1, uint8_t w2, uint8_t w3);
	void set_winter_indent_weeks_4(uint8_t w1, uint8_t w2, uint8_t w3, uint8_t w4);
	void set_winter_indent_weeks_5(uint8_t w1, uint8_t w2, uint8_t w3, uint8_t w4, uint8_t w5);
	void set_spring_indent_weeks(uint8_t w1, uint8_t w2);
	void set_spring_indent_apostol(bool value);
	int8_t winter_indent(int year);
	int8_t spring_indent(int year);
	int8_t apostol_post_length(int year);
	int8_t date_glas(int y, int8_t m, int8_t d, bool julian=true);
	int8_t date_n50(int y, int8_t m, int8_t d, bool julian=true);
	std::optional<std::vector<uint16_t>> date_properties(int y, int8_t m, int8_t d, bool julian=true);
	std::optional<ApEvReads> date_apostol(int y, int8_t m, int8_t d, bool julian=true);
	std::optional<ApEvReads> date_evangelie(int y, int8_t m, int8_t d, bool julian=true);
	std::optional<ApEvReads> resurrect_evangelie(int y, int8_t m, int8_t d, bool julian=true);
	std::optional<year_month_day> get_date_with(int year, uint16_t property, bool julian=true);
	std::optional<std::vector<year_month_day>> get_alldates_with(int year, uint16_t property, bool julian=true);
	std::optional<year_month_day> get_date_withanyof(int year, std::span<uint16_t> properties, bool julian=true);
	std::optional<year_month_day> get_date_withallof(int year, std::span<uint16_t> properties, bool julian=true);
	std::optional<std::vector<year_month_day>> get_alldates_withanyof(int year, std::span<uint16_t> properties, bool julian=true);
	std::string get_description_for_date(int year, int8_t month, int8_t day, bool julian=true);
	std::string get_description_for_dates(std::span<year_month_day> d, const std::string& separator, bool julian=true);
};

OrthYear* OrthodoxCalendar::impl::get_from_cache(int y)
{
	auto it = cache.find(y);
	if(it == cache.end()) {
		if(cache.size() >= cache_max_elements) {
			cache.erase(cache_elements_queue.front());
			cache_elements_queue.pop();
		}
		std::vector<int> weekns; weekns.reserve(17); //week numbers
		weekns.push_back(zimn_otstupka_n1);
		for(int i : zimn_otstupka_n2) weekns.push_back(i);
		for(int i : zimn_otstupka_n3) weekns.push_back(i);
		for(int i : zimn_otstupka_n4) weekns.push_back(i);
		for(int i : zimn_otstupka_n5) weekns.push_back(i);
		for(int i : osen_otstupka)    weekns.push_back(i);
		auto [it1, ok] = cache.insert({y, std::make_unique<OrthYear>(y, weekns, osen_otstupka_apostol)});
		if(!ok) return nullptr;
		it = it1;
		cache_elements_queue.push(y);
	}
	return it->second.get();
}

OrthYear* OrthodoxCalendar::impl::get_from_cache(int& y, int8_t& m, int8_t& d, const bool julian)
{
	if(!julian) {
		auto ymd = OrthodoxCalendar::grigorian_to_julian(y, m, d);
		y = ymd.year;
		m = ymd.month;
		d = ymd.day;
	}
	std::cout<<y<<std::endl;
	return get_from_cache(y);
}

void OrthodoxCalendar::impl::set_cache_size(size_t sz)
{
	cache_max_elements = sz;
	while( cache.size() > sz ) {
		cache.erase(cache_elements_queue.front());
		cache_elements_queue.pop();
	}
}

void OrthodoxCalendar::impl::set_winter_indent_weeks_1(uint8_t w1)
{
	if(w1 != zimn_otstupka_n1) {
		zimn_otstupka_n1 = w1;
		cache.clear();
		while(!cache_elements_queue.empty()) cache_elements_queue.pop();
	}
}

void OrthodoxCalendar::impl::set_winter_indent_weeks_2(uint8_t w1, uint8_t w2)
{
	std::array<uint8_t,2> x {w1, w2};
	if( !std::equal(zimn_otstupka_n2.begin(), zimn_otstupka_n2.end(), x.begin()) ) {
		std::copy(x.begin(), x.end(), zimn_otstupka_n2.begin());
		cache.clear();
		while(!cache_elements_queue.empty()) cache_elements_queue.pop();
	}
}

void OrthodoxCalendar::impl::set_winter_indent_weeks_3(uint8_t w1, uint8_t w2, uint8_t w3)
{
	std::array<uint8_t,3> x {w1, w2, w3};
	if( !std::equal(zimn_otstupka_n3.begin(), zimn_otstupka_n3.end(), x.begin()) ) {
		std::copy(x.begin(), x.end(), zimn_otstupka_n3.begin());
		cache.clear();
		while(!cache_elements_queue.empty()) cache_elements_queue.pop();
	}
}

void OrthodoxCalendar::impl::set_winter_indent_weeks_4(uint8_t w1, uint8_t w2, uint8_t w3, uint8_t w4)
{
	std::array<uint8_t,4> x {w1, w2, w3, w4};
	if( !std::equal(zimn_otstupka_n4.begin(), zimn_otstupka_n4.end(), x.begin()) ) {
		std::copy(x.begin(), x.end(), zimn_otstupka_n4.begin());
		cache.clear();
		while(!cache_elements_queue.empty()) cache_elements_queue.pop();
	}
}

void OrthodoxCalendar::impl::set_winter_indent_weeks_5(uint8_t w1, uint8_t w2, uint8_t w3, uint8_t w4, uint8_t w5)
{
	std::array<uint8_t,5> x {w1, w2, w3, w4, w5};
	if( !std::equal(zimn_otstupka_n5.begin(), zimn_otstupka_n5.end(), x.begin()) ) {
		std::copy(x.begin(), x.end(), zimn_otstupka_n5.begin());
		cache.clear();
		while(!cache_elements_queue.empty()) cache_elements_queue.pop();
	}
}

void OrthodoxCalendar::impl::set_spring_indent_weeks(uint8_t w1, uint8_t w2)
{
	std::array<uint8_t,2> x {w1, w2};
	if( !std::equal(osen_otstupka.begin(), osen_otstupka.end(), x.begin()) ) {
		std::copy(x.begin(), x.end(), osen_otstupka.begin());
		cache.clear();
		while(!cache_elements_queue.empty()) cache_elements_queue.pop();
	}
}

void OrthodoxCalendar::impl::set_spring_indent_apostol(bool value)
{
	if(value != osen_otstupka_apostol) {
		osen_otstupka_apostol = value;
		cache.clear();
		while(!cache_elements_queue.empty()) cache_elements_queue.pop();
	}
}

int8_t OrthodoxCalendar::impl::winter_indent(int year)
{
	if(OrthYear* p = get_from_cache(year); p) {
		return p->get_winter_indent() ;
	} else {
		return std::numeric_limits<int8_t>::max();
	}
}

int8_t OrthodoxCalendar::impl::spring_indent(int year)
{
	if(OrthYear* p = get_from_cache(year); p) {
		return p->get_spring_indent() ;
	} else {
		return std::numeric_limits<int8_t>::max();
	}
}

int8_t OrthodoxCalendar::impl::apostol_post_length(int year)
{
	auto dec_date_by_one = [](int& y, int8_t& m, int8_t& d)
	{
		d--;
		if(d < 1) {
			m--;
			if(m<1) {
				y--;
				m = 12;
			}
			d += OrthodoxCalendar::month_length(m, OrthodoxCalendar::is_leap_year(y, true));
		}
	};
	if(OrthYear* p = get_from_cache(year); p) {
		auto d1 = p->get_date_with(oxc::ned1_po50);
		auto d2 = p->get_date_with(oxc::m6d29);
		if(d1 && d2) {
			int days_count{};
			do {
				dec_date_by_one(year, d2->first, d2->second);
				days_count++;
			} while(*d1 != *d2);
			return days_count-1;
		}
	}
	return 0;
}

int8_t OrthodoxCalendar::impl::date_glas(int y, int8_t m, int8_t d, bool julian)
{
	if(OrthYear* p = get_from_cache(y, m, d, julian); p) {
		return p->get_date_glas(m, d);
	} else {
		return -1;
	}
}

int8_t OrthodoxCalendar::impl::date_n50(int y, int8_t m, int8_t d, bool julian)
{
	if(OrthYear* p = get_from_cache(y, m, d, julian); p) {
		return p->get_date_n50(m, d);
	} else {
		return -1;
	}
}

std::optional<std::vector<uint16_t>> OrthodoxCalendar::impl::date_properties(int y, int8_t m, int8_t d, bool julian)
{
	if(OrthYear* p = get_from_cache(y, m, d, julian); p) {
		return p->get_date_properties(m, d);
	} else {
		return std::nullopt;
	}
}

std::optional<ApEvReads> OrthodoxCalendar::impl::date_apostol(int y, int8_t m, int8_t d, bool julian)
{
	if(OrthYear* p = get_from_cache(y, m, d, julian); p) {
		auto res = p->get_date_apostol(m, d);
		if(res.n < 1) return std::nullopt;
		return res;
	} else {
		return std::nullopt;
	}
}

std::optional<ApEvReads> OrthodoxCalendar::impl::date_evangelie(int y, int8_t m, int8_t d, bool julian)
{
	if(OrthYear* p = get_from_cache(y, m, d, julian); p) {
		auto res = p->get_date_evangelie(m, d);
		if(res.n < 1) return std::nullopt;
		return res;
	} else {
		return std::nullopt;
	}
}

std::optional<ApEvReads> OrthodoxCalendar::impl::resurrect_evangelie(int y, int8_t m, int8_t d, bool julian)
{
	if(OrthYear* p = get_from_cache(y, m, d, julian); p) {
		auto res = p->get_resurrect_evangelie(m, d);
		if(res.n < 1) return std::nullopt;
		return res;
	} else {
		return std::nullopt;
	}
}

std::optional<year_month_day> OrthodoxCalendar::impl::get_date_with(int year, uint16_t property, bool julian)
{
	if(OrthYear* p = get_from_cache(year); p) {
		if(auto x = p->get_date_with(property); x) {
			if(julian) return year_month_day{year, x->first, x->second};
			else return OrthodoxCalendar::julian_to_grigorian(year, x->first, x->second);
		} else {
			return std::nullopt;
		}
	} else {
		return std::nullopt;
	}
}

std::optional<std::vector<year_month_day>> OrthodoxCalendar::impl::get_alldates_with(int year, uint16_t property, bool julian)
{
	if(OrthYear* p = get_from_cache(year); p) {
		if(auto x = p->get_alldates_with(property); x) {
			std::vector<year_month_day> res (x->size());
			if(julian) {
				std::transform(x->begin(), x->end(), res.begin(), [year](const auto& e){
					return year_month_day{year, e.first, e.second};
				});
			} else {
				std::transform(x->begin(), x->end(), res.begin(), [year](const auto& e){
					return OrthodoxCalendar::julian_to_grigorian(year, e.first, e.second);
				});
			}
			return res;
		} else {
			return std::nullopt;
		}
	} else {
		return std::nullopt;
	}
}

std::optional<year_month_day> OrthodoxCalendar::impl::get_date_withanyof(int year, std::span<uint16_t> properties, bool julian)
{
	if(OrthYear* p = get_from_cache(year); p) {
		if(auto x = p->get_date_withanyof(properties); x) {
			if(julian) return year_month_day{year, x->first, x->second};
			else return OrthodoxCalendar::julian_to_grigorian(year, x->first, x->second);
		} else {
			return std::nullopt;
		}
	} else {
		return std::nullopt;
	}
}

std::optional<year_month_day> OrthodoxCalendar::impl::get_date_withallof(int year, std::span<uint16_t> properties, bool julian)
{
	if(OrthYear* p = get_from_cache(year); p) {
		if(auto x = p->get_date_withallof(properties); x) {
			if(julian) return year_month_day{year, x->first, x->second};
			else return OrthodoxCalendar::julian_to_grigorian(year, x->first, x->second);
		} else {
			return std::nullopt;
		}
	} else {
		return std::nullopt;
	}
}

std::optional<std::vector<year_month_day>> OrthodoxCalendar::impl::get_alldates_withanyof(int year, std::span<uint16_t> properties, bool julian)
{
	if(OrthYear* p = get_from_cache(year); p) {
		if(auto x = p->get_alldates_withanyof(properties); x) {
			std::vector<year_month_day> res (x->size());
			if(julian) {
				std::transform(x->begin(), x->end(), res.begin(), [year](const auto& e){
					return year_month_day{year, e.first, e.second};
				});
			} else {
				std::transform(x->begin(), x->end(), res.begin(), [year](const auto& e){
					return OrthodoxCalendar::julian_to_grigorian(year, e.first, e.second);
				});
			}
			return res;
		} else {
			return std::nullopt;
		}
	} else {
		return std::nullopt;
	}
}

std::string OrthodoxCalendar::impl::get_description_for_date(int year, int8_t month, int8_t day, bool julian)
{
	if(OrthYear* p = get_from_cache(year, month, day, julian); p) {
		return p->get_description_forday(month, day);
	} else {
		return {};
	}
}

std::string OrthodoxCalendar::impl::get_description_for_dates(std::span<year_month_day> d, const std::string& separator, bool julian)
{
	std::string res;
	for(const auto& [yy, mm, dd]: d) {
		if(auto s = get_description_for_date(yy, mm, dd, julian); !s.empty())
			res += s + separator;
	}
	return res;
}

/*------------------------------------------------*/
/*     class OrthodoxCalendar Static Methods      */
/*------------------------------------------------*/

year_month_day OrthodoxCalendar::pascha(int year, bool julian)
{ //using Gauss method
	if(year<1) return {};
	int8_t m_=3, p;
	int a, b, c, d, e;
	a = year % 19;
	b = year % 4;
	c = year % 7;
	d = (19*a+15) % 30;
	e = (2*b+4*c+6*d+6) % 7;
	p = 22 + d + e;
	if(p>31) {
		p = d + e - 9;
		m_ = 4;
	}
	if(julian) return { year, m_, p };
	else return julian_to_grigorian( year, m_, p );
}

bool OrthodoxCalendar::is_leap_year(int y, bool julian)
{
	if(julian) return y%4 == 0 ;
	else return (y%400 == 0) || (y%100 != 0 && y%4 == 0) ;
}

int8_t OrthodoxCalendar::month_length(int8_t month, bool leap)
{
	int8_t k{};
	switch(month) {
		case 1: { k = 31; }
				break;
		case 2: { k = (leap ? 29 : 28); }
				break;
		case 3: { k = 31; }
				break;
		case 4: { k = 30; }
				break;
		case 5: { k = 31; }
				break;
		case 6: { k = 30; }
				break;
		case 7: { k = 31; }
				break;
		case 8: { k = 31; }
				break;
		case 9: { k = 30; }
				break;
		case 10:{ k = 31; }
				break;
		case 11:{ k = 30; }
				break;
		case 12:{ k = 31; }
	}
	return k;
}

uint64_t OrthodoxCalendar::jdn_for_date(int y, int8_t m, int8_t d, bool julian)
{
	if(y<1) return 0;
	if(julian) {
		uint64_t a = (14 - m) / 12;
		uint64_t b = y + 4800 - a;
		uint64_t c = m + 12 * a - 3;
		uint64_t x1 = (153 * c + 2) / 5;
		uint64_t x2 = b / 4;
		return (d + x1 + 365 * b + x2 - 32083);
	} else {
		uint64_t a = (14 - m) / 12;
		uint64_t b = y + 4800 - a;
		uint64_t c = m + 12 * a - 3;
		uint64_t x1 = (153 * c + 2) / 5;
		uint64_t x2 = b / 4;
		uint64_t x3 = b / 100;
		uint64_t x4 = b / 400;
		return (d + x1 + 365 * b + x2 - x3 + x4 - 32045);
	}
}

uint64_t OrthodoxCalendar::jdn_for_date(year_month_day d, bool julian)
{
	return jdn_for_date(d.year, d.month, d.day, julian);
}

year_month_day OrthodoxCalendar::grigorian_to_julian(int y, int8_t m, int8_t d)
{
  uint64_t a = 32082 + jdn_for_date(y, m, d, false);
  uint64_t b = (4*a + 3) / 1461;
  uint64_t c = (1461*b) / 4 ;
  c = a - c;
  uint64_t x1 = (5*c + 2) / 153;
  d = (153*x1 + 2) / 5;
  d = c - d + 1;
  y = m = x1 / 10;
  m = x1 + 3 - 12*m;
  y = b - 4800 + y;
  return {y, m, d};
}

year_month_day OrthodoxCalendar::julian_to_grigorian(int y, int8_t m, int8_t d)
{
  uint64_t a = 32044 + jdn_for_date(y, m, d);
  uint64_t b = (4*a + 3) / 146097;
  uint64_t c = (146097*b) / 4 ;
  c = a - c;
  uint64_t x1 = (4*c + 3) / 1461;
  uint64_t x2 = (1461*x1) / 4;
  x2 = c - x2;
  uint64_t x3 = (5*x2 + 2) / 153;
  d = (153*x3 +2) / 5;
  d = x2 - d + 1;
  y = m = x3 / 10;
  m = x3 + 3 - 12*m;
  y = 100*b + x1 - 4800 + y;
  return {y, m, d};
}

int8_t OrthodoxCalendar::weekday_for_date(int y, int8_t m, int8_t d, bool julian)
{ // return: 0-вс, 1-пн, 2-вт, 3-ср, 4-чт, 5-пт, 6-сб.
	if(y<1) return -1;
	switch(jdn_for_date(y, m, d, julian) % 7) {
		case 0: { return 1; }
		case 1: { return 2; }
		case 2: { return 3; }
		case 3: { return 4; }
		case 4: { return 5; }
		case 5: { return 6; }
		case 6: { return 0; }
		default: { return -1; }
	};
}

/*----------------------------------------------*/
/*          class OrthodoxCalendar              */
/*----------------------------------------------*/

OrthodoxCalendar::OrthodoxCalendar() : pimpl(std::make_unique<impl>())
{
}

void OrthodoxCalendar::set_cache_size(size_t sz)
{
	return pimpl->set_cache_size(sz);
}

void OrthodoxCalendar::set_winter_indent_weeks_1(uint8_t w1)
{
	return pimpl->set_winter_indent_weeks_1(w1);
}

void OrthodoxCalendar::set_winter_indent_weeks_2(uint8_t w1, uint8_t w2)
{
	return pimpl->set_winter_indent_weeks_2(w1, w2);
}

void OrthodoxCalendar::set_winter_indent_weeks_3(uint8_t w1, uint8_t w2, uint8_t w3)
{
	return pimpl->set_winter_indent_weeks_3(w1, w2, w3);
}

void OrthodoxCalendar::set_winter_indent_weeks_4(uint8_t w1, uint8_t w2, uint8_t w3, uint8_t w4)
{
	return pimpl->set_winter_indent_weeks_4(w1, w2, w3, w4);
}

void OrthodoxCalendar::set_winter_indent_weeks_5(uint8_t w1, uint8_t w2, uint8_t w3, uint8_t w4, uint8_t w5)
{
	return pimpl->set_winter_indent_weeks_5(w1, w2, w3, w4, w5);
}

void OrthodoxCalendar::set_spring_indent_weeks(uint8_t w1, uint8_t w2)
{
	return pimpl->set_spring_indent_weeks(w1, w2);
}

void OrthodoxCalendar::set_spring_indent_apostol(bool value)
{
	return pimpl->set_spring_indent_apostol(value);
}

int8_t OrthodoxCalendar::winter_indent(int year)
{
	return pimpl->winter_indent(year);
}

int8_t OrthodoxCalendar::spring_indent(int year)
{
	return pimpl->spring_indent(year);
}

int8_t OrthodoxCalendar::apostol_post_length(int year)
{
	return pimpl->apostol_post_length(year);
}

int8_t OrthodoxCalendar::date_glas(int y, int8_t m, int8_t d, bool julian)
{
	return pimpl->date_glas(y, m, d, julian);
}

int8_t OrthodoxCalendar::date_n50(int y, int8_t m, int8_t d, bool julian)
{
	return pimpl->date_n50(y, m, d, julian);
}

std::optional<std::vector<uint16_t>> OrthodoxCalendar::date_properties(int y, int8_t m, int8_t d, bool julian)
{
	return pimpl->date_properties(y, m, d, julian);
}

std::optional<ApEvReads> OrthodoxCalendar::date_apostol(int y, int8_t m, int8_t d, bool julian)
{
	return pimpl->date_apostol(y, m, d, julian);
}

std::optional<ApEvReads> OrthodoxCalendar::date_evangelie(int y, int8_t m, int8_t d, bool julian)
{
	return pimpl->date_evangelie(y, m, d, julian);
}

std::optional<ApEvReads> OrthodoxCalendar::resurrect_evangelie(int y, int8_t m, int8_t d, bool julian)
{
	return pimpl->resurrect_evangelie(y, m, d, julian);
}

std::optional<year_month_day> OrthodoxCalendar::get_date_with(int year, uint16_t property, bool julian)
{
	return pimpl->get_date_with(year, property, julian);
}

std::optional<std::vector<year_month_day>> OrthodoxCalendar::get_alldates_with(int year, uint16_t property, bool julian)
{
	return pimpl->get_alldates_with(year, property, julian);
}

std::optional<year_month_day> OrthodoxCalendar::get_date_withanyof(int year, std::span<uint16_t> properties, bool julian)
{
	return pimpl->get_date_withanyof(year, properties, julian);
}

std::optional<year_month_day> OrthodoxCalendar::get_date_withallof(int year, std::span<uint16_t> properties, bool julian)
{
	return pimpl->get_date_withallof(year, properties, julian);
}

std::optional<std::vector<year_month_day>> OrthodoxCalendar::get_alldates_withanyof(int year, std::span<uint16_t> properties, bool julian)
{
	return pimpl->get_alldates_withanyof(year, properties, julian);
}

std::string OrthodoxCalendar::get_description_for_date(int year, int8_t month, int8_t day, bool julian)
{
	return pimpl->get_description_for_date(year, month, day, julian);
}

std::string OrthodoxCalendar::get_description_for_dates(std::span<year_month_day> days, bool julian, const std::string separator)
{
	return pimpl->get_description_for_dates(days, separator, julian);
}

OrthodoxCalendar::~OrthodoxCalendar() = default;

OrthodoxCalendar::OrthodoxCalendar(OrthodoxCalendar&&) = default;

OrthodoxCalendar& OrthodoxCalendar::operator=(OrthodoxCalendar&&) = default;

} //namespace oxc

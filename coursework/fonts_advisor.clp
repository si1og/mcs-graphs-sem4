(deftemplate known
   (slot attribute (type SYMBOL))
   (slot value (type SYMBOL))
)

; обработка некорректного ввода
(deffunction ask-choice-1-2 ()
   (bind ?ans 0)

   (while (and (neq ?ans 1) (neq ?ans 2)) do
      (printout t "Ваш выбор (введите 1 или 2): ")
      (bind ?ans (read))

      (if (and (neq ?ans 1) (neq ?ans 2))
         then
            (printout t crlf "Ошибка! Можно ввести только 1 или 2. Попробуйте ещё раз." crlf)
      )
   )

   (return ?ans)
)

; универсальная функция вопроса
(deffunction ask-binary (?attribute ?question)
   (printout t crlf)
   (printout t ?question crlf)
   (printout t "1) Нет" crlf)
   (printout t "2) Да" crlf)

   (bind ?ans (ask-choice-1-2))

   (if (= ?ans 1)
      then
         (assert (known (attribute ?attribute) (value no)))
      else
         (assert (known (attribute ?attribute) (value yes)))
   )
)

; функция вывода результата
(deffunction print-result (?number $?items)
   (printout t crlf "Лист " ?number crlf)

   (foreach ?item ?items
      (printout t "- " ?item crlf)
   )

   (halt)
)

; -------------------------
; ВОПРОСЫ
; -------------------------

(defrule ask-additional-text
   (not (known (attribute additional-text)))
   =>
   (ask-binary additional-text
      "Требуется ли размещение дополнительного текста помимо имени и логотипа?")
)

(defrule ask-cyrillic
   (known (attribute additional-text) (value yes))
   (not (known (attribute cyrillic)))
   =>
   (ask-binary cyrillic
      "Есть ли кириллица?")
)

(defrule ask-reading-distance
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic))
   (not (known (attribute read-over-1m)))
   =>
   (ask-binary read-over-1m
      "Требуется ли чтение с расстояния более 1 метра?")
)

(defrule ask-dark-background-without-additional-text
   (known (attribute additional-text) (value no))
   (not (known (attribute dark-background)))
   =>
   (ask-binary dark-background
      "Фон бейджа тёмный?")
)

(defrule ask-dark-background-with-additional-text
   (known (attribute additional-text) (value yes))
   (known (attribute read-over-1m) (value yes))
   (not (known (attribute dark-background)))
   =>
   (ask-binary dark-background
      "Фон бейджа тёмный?")
)

(defrule ask-lamination-without-additional-text
   (known (attribute additional-text) (value no))
   (known (attribute dark-background))
   (not (known (attribute lamination)))
   =>
   (ask-binary lamination
      "Есть ли ламинация?")
)

(defrule ask-lamination-with-additional-text
   (known (attribute additional-text) (value yes))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background))
   (not (known (attribute lamination)))
   =>
   (ask-binary lamination
      "Есть ли ламинация?")
)

(defrule ask-paper-density
   (known (attribute additional-text) (value yes))
   (known (attribute read-over-1m) (value no))
   (not (known (attribute paper-density-over-250)))
   =>
   (ask-binary paper-density-over-250
      "Плотность бумаги выше 250 г/м²?")
)

; -------------------------
; ЛИСТЬЯ 1-4
; Нет дополнительного текста
; -------------------------

(defrule node-1
   (known (attribute additional-text) (value no))
   (known (attribute dark-background) (value no))
   (known (attribute lamination) (value no))
   =>
   (print-result 1
      "гротеск или антиква"
      "Regular / SemiBold"
      "допускаются более тонкие элементы"
      "стандартный межбуквенный интервал"
      "достаточный контраст текста и фона")
)

(defrule node-2
   (known (attribute additional-text) (value no))
   (known (attribute dark-background) (value no))
   (known (attribute lamination) (value yes))
   =>
   (print-result 2
      "гротеск или антиква"
      "SemiBold"
      "увеличенная высота строчных знаков"
      "достаточный контраст текста и фона")
)

(defrule node-3
   (known (attribute additional-text) (value no))
   (known (attribute dark-background) (value yes))
   (known (attribute lamination) (value no))
   =>
   (print-result 3
      "гротеск"
      "SemiBold / Bold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона")
)

(defrule node-4
   (known (attribute additional-text) (value no))
   (known (attribute dark-background) (value yes))
   (known (attribute lamination) (value yes))
   =>
   (print-result 4
      "гротеск"
      "Bold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона"
      "избегать тонких штрихов")
)

; -------------------------
; ЛИСТЬЯ 5-10
; Дополнительный текст есть, кириллицы нет
; -------------------------

(defrule node-5
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value no))
   (known (attribute read-over-1m) (value no))
   (known (attribute paper-density-over-250) (value no))
   =>
   (print-result 5
      "гротеск"
      "латиница"
      "Regular / SemiBold"
      "простые открытые формы символов"
      "избегать тонких деталей")
)

(defrule node-6
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value no))
   (known (attribute read-over-1m) (value no))
   (known (attribute paper-density-over-250) (value yes))
   =>
   (print-result 6
      "антиква или гротеск"
      "латиница"
      "Regular"
      "допускаются тонкие элементы"
      "стандартный межбуквенный интервал")
)

(defrule node-7
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value no))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background) (value no))
   (known (attribute lamination) (value no))
   =>
   (print-result 7
      "гротеск"
      "латиница"
      "Regular / SemiBold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона")
)

(defrule node-8
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value no))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background) (value no))
   (known (attribute lamination) (value yes))
   =>
   (print-result 8
      "гротеск"
      "латиница"
      "SemiBold"
      "увеличенная высота строчных знаков"
      "избегать тонких штрихов"
      "высокий контраст текста и фона")
)

(defrule node-9
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value no))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background) (value yes))
   (known (attribute lamination) (value no))
   =>
   (print-result 9
      "гротеск"
      "латиница"
      "SemiBold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона")
)

(defrule node-10
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value no))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background) (value yes))
   (known (attribute lamination) (value yes))
   =>
   (print-result 10
      "гротеск"
      "латиница"
      "Bold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона"
      "избегать тонких штрихов"
      "увеличенный межбуквенный интервал")
)

; -------------------------
; ЛИСТЬЯ 11-16
; Дополнительный текст есть, кириллица есть
; -------------------------

(defrule node-11
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value yes))
   (known (attribute read-over-1m) (value no))
   (known (attribute paper-density-over-250) (value no))
   =>
   (print-result 11
      "гротеск"
      "кириллица"
      "SemiBold"
      "простые открытые формы символов"
      "увеличенная высота строчных знаков"
      "избегать тонких деталей"
      "слегка увеличенный межбуквенный интервал")
)

(defrule node-12
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value yes))
   (known (attribute read-over-1m) (value no))
   (known (attribute paper-density-over-250) (value yes))
   =>
   (print-result 12
      "гуманистический гротеск"
      "кириллица"
      "SemiBold"
      "средний или высокий контраст штрихов"
      "увеличенная высота строчных знаков"
      "умеренно увеличенный межбуквенный интервал"
      "без тонких декоративных элементов")
)

(defrule node-13
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value yes))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background) (value no))
   (known (attribute lamination) (value no))
   =>
   (print-result 13
      "гротеск"
      "кириллица"
      "SemiBold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона"
      "слегка увеличенный межбуквенный интервал")
)

(defrule node-14
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value yes))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background) (value no))
   (known (attribute lamination) (value yes))
   =>
   (print-result 14
      "гротеск"
      "кириллица"
      "SemiBold / Bold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона"
      "избегать тонких штрихов"
      "слегка увеличенный межбуквенный интервал")
)

(defrule node-15
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value yes))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background) (value yes))
   (known (attribute lamination) (value no))
   =>
   (print-result 15
      "гротеск"
      "кириллица"
      "Bold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона"
      "слегка увеличенный межбуквенный интервал")
)

(defrule node-16
   (known (attribute additional-text) (value yes))
   (known (attribute cyrillic) (value yes))
   (known (attribute read-over-1m) (value yes))
   (known (attribute dark-background) (value yes))
   (known (attribute lamination) (value yes))
   =>
   (print-result 16
      "гротеск"
      "кириллица"
      "Bold"
      "увеличенная высота строчных знаков"
      "высокий контраст текста и фона"
      "избегать тонких штрихов"
      "увеличенный межбуквенный интервал")
)

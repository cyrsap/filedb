#ifndef STORAGE_H
#define STORAGE_H

#include <stdlib.h>
#include <stdint.h>

// Функции работы с хранилищем данных

// Инициализация хранилища
int st_init();
// Вывод всех ключей данных в хранилище в строку aOutString
int st_list(char *aOutString, size_t aStrLength);
// Вставка пары Ключ-Значение aKey-aValue в хранилище
int st_put(char *aKey, char *aValue);
// Получение значения по ключу
int st_get(const char *aKey, char *aValue, uint32_t aValueSize);
// Удаление записи из хранилища по ключу
int st_erase(const char *aKey);
// Очистка хранилища
int st_clean();
// Дедупликация
int st_deduplicate();

#endif //STORAGE_H

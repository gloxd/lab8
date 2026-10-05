using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace ClassJournalApp
{
    /// <summary>
    /// Сущность, представляющая запись в журнале класса.
    /// </summary>
    [Serializable]
    public class StudentProgress
    {
        public int Id{ get; set; }
        public string FullName{ get; set; } = string.Empty;
        public string Subject{ get; set; } = string.Empty;
        public int Grade{ get; set; }
        public bool IsPresent{ get; set; }

            public StudentProgress() {}

        public StudentProgress(int id, string fullName, string subject, int grade, bool isPresent)
        {
            Id = id;
            FullName = fullName;
            Subject = subject;
            Grade = grade;
            IsPresent = isPresent;
        }

        public override string ToString()
        {
            string presenceStatus = IsPresent ? "Присутствовал" : "Отсутствовал";
            return $"[ID: {Id,2}] {FullName,-20} | Предмет: {Subject,-12} | Оценка: {Grade} | Статус: {presenceStatus}";
        }
    }

    /// <summary>
    /// Вспомогательный класс для работы с БД (бинарным файлом) с использованием LINQ.
    /// </summary>
    public class ClassJournalManager
    {
        private readonly string _filePath;

        public ClassJournalManager(string filePath)
        {
            _filePath = filePath;
        }

        /// <summary>
        /// Запись списка объектов в бинарный файл.
        /// </summary>
        public void SaveToFile(List<StudentProgress> records)
        {
            try
            {
                using (var writer = new BinaryWriter(File.Open(_filePath, FileMode.Create)))
                {
                    writer.Write(records.Count);
                    foreach(var record in records)
                    {
                        writer.Write(record.Id);
                        writer.Write(record.FullName);
                        writer.Write(record.Subject);
                        writer.Write(record.Grade);
                        writer.Write(record.IsPresent);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Ошибка при сохранении файла: {ex.Message}");
            }
        }

        /// <summary>
        /// Чтение базы данных из бинарного файла.
        /// </summary>
        public List<StudentProgress> ReadFromFile()
        {
            var records = new List<StudentProgress>();

            if (!File.Exists(_filePath))
            {
                Console.WriteLine("Файл базы данных не найден. Будет создан новый при сохранении.");
                return records;
            }

            try
            {
                using (var reader = new BinaryReader(File.Open(_filePath, FileMode.Open)))
                {
                    int count = reader.ReadInt32();
                    for (int i = 0; i < count; i++)
                    {
                        var record = new StudentProgress
                        {
                            Id = reader.ReadInt32(),
                            FullName = reader.ReadString(),
                            Subject = reader.ReadString(),
                            Grade = reader.ReadInt32(),
                            IsPresent = reader.ReadBoolean()
                        };
                        records.Add(record);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Ошибка при чтении файла: {ex.Message}");
            }

            return records;
        }

        /// <summary>
        /// Удаление элемента по ID (ключу).
        /// </summary>
        public bool DeleteById(int id)
        {
            List<StudentProgress> records = ReadFromFile();
            StudentProgress ? itemToDelete = records.FirstOrDefault(r = > r.Id == id);

            if (itemToDelete != null)
            {
                records.Remove(itemToDelete);
                SaveToFile(records);
                return true;
            }

            return false;
        }

        /// <summary>
        /// Добавление элемента в базу данных.
        /// </summary>
        public void AddRecord(StudentProgress newRecord)
        {
            List<StudentProgress> records = ReadFromFile();

            // Автоматическая генерация уникального ключа (ID) с помощью LINQ
            int newId = records.Any() ? records.Max(r = > r.Id) + 1 : 1;
            newRecord.Id = newId;

            records.Add(newRecord);
            SaveToFile(records);
        }

        // =========================================================================
        // РЕАЛИЗАЦИЯ LINQ-ЗАПРОСОВ (Пункт 5)
        // =========================================================================

        /// <summary>
        /// Запрос 1 (Список): Получить перечень всех отличников (оценка 5), присутствовавших на занятии.
        /// </summary>
        public List<StudentProgress> GetPresentExcellentStudents()
        {
            List<StudentProgress> records = ReadFromFile();

            return (from record in records
                where record.Grade == 5 && record.IsPresent
                select record).ToList();
        }

        /// <summary>
        /// Запрос 2 (Список): Получить перечень учеников по конкретному предмету, отсортированных по имени.
        /// </summary>
        public List<StudentProgress> GetStudentsBySubject(string subject)
        {
            List<StudentProgress> records = ReadFromFile();

            return records
                .Where(r = > r.Subject.Equals(subject, StringComparison.OrdinalIgnoreCase))
                .OrderBy(r = > r.FullName)
                .ToList();
        }

        /// <summary>
        /// Запрос 3 (Одно значение): Рассчитать средний балл класса по всем предметам.
        /// </summary>
        public double GetAverageGrade()
        {
            List<StudentProgress> records = ReadFromFile();

            if (!records.Any())
            {
                return 0.0;
            }

            return records.Average(r = > r.Grade);
        }

        /// <summary>
        /// Запрос 4 (Одно значение): Получить общее количество пропущенных занятий.
        /// </summary>
        public int GetTotalAbsencesCount()
        {
            List<StudentProgress> records = ReadFromFile();

            return records.Count(r = > !r.IsPresent);
        }
    }

    internal class Program
    {
        private const string DbFileName = "journal_db.dat";

        private static void Main()
        {
            var manager = new ClassJournalManager(DbFileName);
            InitializeSeedDataIfEmpty(manager);

            bool isRunning = true;

            while (isRunning)
            {
                Console.Clear();
                Console.WriteLine("==========================================");
                Console.WriteLine("    ЖУРНАЛ КЛАССА - БАЗА ДАННЫХ (БИНАРНАЯ)");
                Console.WriteLine("==========================================");
                Console.WriteLine("1. Просмотр всей базы данных");
                Console.WriteLine("2. Чтение/Перезагрузка БД из файла");
                Console.WriteLine("3. Добавить запись");
                Console.WriteLine("4. Удалить запись по ID (ключу)");
                Console.WriteLine("5. [Запрос 1] Отличники, бывшие на уроке (Список)");
                Console.WriteLine("6. [Запрос 2] Список учеников по предмету (Список)");
                Console.WriteLine("7. [Запрос 3] Средний балл класса (Одно значение)");
                Console.WriteLine("8. [Запрос 4] Общее количество пропусков (Одно значение)");
                Console.WriteLine("0. Выход");
                Console.WriteLine("==========================================");
                Console.Write("Выберите пункт меню: ");

                string ? choice = Console.ReadLine();
                Console.WriteLine();

                switch (choice)
                {
                case "1":
                case "2":
                    ShowDatabase(manager);
                    break;
                case "3":
                    AddNewRecord(manager);
                    break;
                case "4":
                    DeleteRecord(manager);
                    break;
                case "5":
                    ShowPresentExcellentStudents(manager);
                    break;
                case "6":
                    ShowStudentsBySubject(manager);
                    break;
                case "7":
                    ShowAverageGrade(manager);
                    break;
                case "8":
                    ShowTotalAbsences(manager);
                    break;
                case "0":
                    isRunning = false;
                    Console.WriteLine("Завершение работы программы...");
                    break;
                default:
                    Console.WriteLine("Неверный ввод, нажмите Enter для повтора.");
                    break;
                }

                if (isRunning)
                {
                    Console.WriteLine("\nНажмите Enter, чтобы продолжить...");
                    Console.ReadLine();
                }
            }
        }

        private static void ShowDatabase(ClassJournalManager manager)
        {
            List<StudentProgress> records = manager.ReadFromFile();

            if (records.Count == 0)
            {
                Console.WriteLine("База данных пуста.");
                return;
            }

            Console.WriteLine("--- Содержимое базы данных ---");
            foreach(var record in records)
            {
                Console.WriteLine(record);
            }
        }

        private static void AddNewRecord(ClassJournalManager manager)
        {
            Console.WriteLine("--- Добавление новой записи ---");

            Console.Write("Введите ФИО ученика: ");
            string fullName = Console.ReadLine() ? ? "Неизвестно";

            Console.Write("Введите предмет: ");
            string subject = Console.ReadLine() ? ? "Общий";

            int grade = ReadIntFromConsole("Введите оценку (2-5): ", 2, 5);
            bool isPresent = ReadBoolFromConsole("Ученик присутствует? (1 - Да, 0 - Нет): ");

            var newRecord = new StudentProgress(0, fullName, subject, grade, isPresent);
            manager.AddRecord(newRecord);

            Console.WriteLine("Запись успешно добавлена!");
        }

        private static void DeleteRecord(ClassJournalManager manager)
        {
            Console.WriteLine("--- Удаление записи ---");
            int id = ReadIntFromConsole("Введите ID записи для удаления: ", 1, int.MaxValue);

            if (manager.DeleteById(id))
            {
                Console.WriteLine($"Запись с ID {id} успешно удалена.");
            }
            else
            {
                Console.WriteLine($"Запись с ID {id} не найдена.");
            }
        }

        private static void ShowPresentExcellentStudents(ClassJournalManager manager)
        {
            Console.WriteLine("--- Отличники на уроке ---");
            List<StudentProgress> result = manager.GetPresentExcellentStudents();

            if (result.Count == 0)
            {
                Console.WriteLine("Отличники, присутствующие на занятии, не найдены.");
                return;
            }

            foreach(var student in result)
            {
                Console.WriteLine(student);
            }
        }

        private static void ShowStudentsBySubject(ClassJournalManager manager)
        {
            Console.Write("Введите название предмета для поиска (например, Математика): ");
            string subject = Console.ReadLine() ? ? string.Empty;

            List<StudentProgress> result = manager.GetStudentsBySubject(subject);

            Console.WriteLine($"\n--- Список учеников по предмету \"{subject}\" ---");
            if (result.Count == 0)
            {
                Console.WriteLine("Записи по данному предмету не найдены.");
                return;
            }

            foreach(var student in result)
            {
                Console.WriteLine(student);
            }
        }

        private static void ShowAverageGrade(ClassJournalManager manager)
        {
            double average = manager.GetAverageGrade();
            Console.WriteLine($"Средний балл по всей базе данных: {average:F2}");
        }

        private static void ShowTotalAbsences(ClassJournalManager manager)
        {
            int absences = manager.GetTotalAbsencesCount();
            Console.WriteLine($"Общее количество пропущенных занятий: {absences}");
        }

        // Вспомогательные методы ввода с обработкой ошибок
        private static int ReadIntFromConsole(string prompt, int min, int max)
        {
            while (true)
            {
                Console.Write(prompt);
                if (int.TryParse(Console.ReadLine(), out int value) && value >= min && value <= max)
                {
                    return value;
                }
                Console.WriteLine($"Ошибка ввода. Введите целое число от {min} до {max}.");
            }
        }

        private static bool ReadBoolFromConsole(string prompt)
        {
            while (true)
            {
                Console.Write(prompt);
                string ? input = Console.ReadLine() ? .Trim();
                if (input == "1" || input ? .Equals("да", StringComparison.OrdinalIgnoreCase) == true)
                {
                    return true;
                }
                if (input == "0" || input ? .Equals("нет", StringComparison.OrdinalIgnoreCase) == true)
                {
                    return false;
                }
                Console.WriteLine("Ошибка ввода. Введите 1 (Да) или 0 (Нет).");
            }
        }

        // Первоначальное заполнение при первом запуске
        private static void InitializeSeedDataIfEmpty(ClassJournalManager manager)
        {
            if (!File.Exists(DbFileName))
            {
                var initialData = new List<StudentProgress>
                {
                    new StudentProgress(1, "Иванов И.И.", "Математика", 5, true),
                    new StudentProgress(2, "Петров П.П.", "Физика", 3, false),
                    new StudentProgress(3, "Сидоров С.С.", "Математика", 5, true),
                    new StudentProgress(4, "Смирнова А.В.", "История", 4, true),
                    new StudentProgress(5, "Кузнецов Н.А.", "Физика", 2, false)
                };
                manager.SaveToFile(initialData);
            }
        }
    }
} "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

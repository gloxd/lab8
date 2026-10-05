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
        public int Id { get; set; }
        public string FullName { get; set; }
        public string Subject { get; set; }
        public int Grade { get; set; }
        public bool IsPresent { get; set; }

        public StudentProgress()
        {
            FullName = string.Empty;
            Subject = string.Empty;
        }

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

        public void SaveToFile(List<StudentProgress> records)
        {
            try
            {
                using (BinaryWriter writer = new BinaryWriter(File.Open(_filePath, FileMode.Create)))
                {
                    writer.Write(records.Count);
                    foreach (StudentProgress record in records)
                    {
                        writer.Write(record.Id);
                        writer.Write(record.FullName ?? string.Empty);
                        writer.Write(record.Subject ?? string.Empty);
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

        public List<StudentProgress> ReadFromFile()
        {
            List<StudentProgress> records = new List<StudentProgress>();

            if (!File.Exists(_filePath))
            {
                Console.WriteLine("Файл базы данных не найден. Будет создан новый при сохранении.");
                return records;
            }

            try
            {
                using (BinaryReader reader = new BinaryReader(File.Open(_filePath, FileMode.Open)))
                {
                    int count = reader.ReadInt32();
                    for (int i = 0; i < count; i++)
                    {
                        StudentProgress record = new StudentProgress
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

        public bool DeleteById(int id)
        {
            List<StudentProgress> records = ReadFromFile();

            // Ищем нужную запись по ID
            StudentProgress itemToDelete = records.FirstOrDefault(r => r.Id == id);

            if (itemToDelete != null)
            {
                records.Remove(itemToDelete);
                SaveToFile(records);
                return true;
            }

            return false;
        }

        public void AddRecord(StudentProgress newRecord)
        {
            List<StudentProgress> records = ReadFromFile();

            int newId = records.Any() ? records.Max(r => r.Id) + 1 : 1;
            newRecord.Id = newId;

            records.Add(newRecord);
            SaveToFile(records);
        }

        public List<StudentProgress> GetPresentExcellentStudents()
        {
            List<StudentProgress> records = ReadFromFile();

            return (from record in records
                    where record.Grade == 5 && record.IsPresent
                    select record).ToList();
        }

        public List<StudentProgress> GetStudentsBySubject(string subject)
        {
            List<StudentProgress> records = ReadFromFile();

            return records
                .Where(r => string.Equals(r.Subject, subject, StringComparison.OrdinalIgnoreCase))
                .OrderBy(r => r.FullName)
                .ToList();
        }

        public double GetAverageGrade()
        {
            List<StudentProgress> records = ReadFromFile();

            if (!records.Any())
            {
                return 0.0;
            }

            return records.Average(r => r.Grade);
        }

        public int GetTotalAbsencesCount()
        {
            List<StudentProgress> records = ReadFromFile();

            return records.Count(r => !r.IsPresent);
        }
    }

    public class Program
    {
        private const string DbFileName = "journal_db.dat";

        public static void Main(string[] args)
        {
            ClassJournalManager manager = new ClassJournalManager(DbFileName);
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

                string choice = Console.ReadLine();
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
            foreach (StudentProgress record in records)
            {
                Console.WriteLine(record);
            }
        }

        private static void AddNewRecord(ClassJournalManager manager)
        {
            Console.WriteLine("--- Добавление новой записи ---");

            Console.Write("Введите ФИО ученика: ");
            string fullName = Console.ReadLine();
            if (string.IsNullOrEmpty(fullName)) fullName = "Неизвестно";

            Console.Write("Введите предмет: ");
            string subject = Console.ReadLine();
            if (string.IsNullOrEmpty(subject)) subject = "Общий";

            int grade = ReadIntFromConsole("Введите оценку (2-5): ", 2, 5);
            bool isPresent = ReadBoolFromConsole("Ученик присутствует? (1 - Да, 0 - Нет): ");

            StudentProgress newRecord = new StudentProgress(0, fullName, subject, grade, isPresent);
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

            foreach (StudentProgress student in result)
            {
                Console.WriteLine(student);
            }
        }

        private static void ShowStudentsBySubject(ClassJournalManager manager)
        {
            Console.Write("Введите название предмета для поиска (например, Математика): ");
            string subject = Console.ReadLine();
            if (subject == null) subject = string.Empty;

            List<StudentProgress> result = manager.GetStudentsBySubject(subject);

            Console.WriteLine($"\n--- Список учеников по предмету \"{subject}\" ---");
            if (result.Count == 0)
            {
                Console.WriteLine("Записи по данному предмету не найдены.");
                return;
            }

            foreach (StudentProgress student in result)
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

        private static int ReadIntFromConsole(string prompt, int min, int max)
        {
            while (true)
            {
                Console.Write(prompt);
                string input = Console.ReadLine();
                if (int.TryParse(input, out int value) && value >= min && value <= max)
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
                string input = Console.ReadLine();
                if (input != null) input = input.Trim();

                if (input == "1" || string.Equals(input, "да", StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
                if (input == "0" || string.Equals(input, "нет", StringComparison.OrdinalIgnoreCase))
                {
                    return false;
                }
                Console.WriteLine("Ошибка ввода. Введите 1 (Да) или 0 (Нет).");
            }
        }

        private static void InitializeSeedDataIfEmpty(ClassJournalManager manager)
        {
            if (!File.Exists(DbFileName))
            {
                List<StudentProgress> initialData = new List<StudentProgress>
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
}
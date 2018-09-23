#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <Windows.h>
#include <array>
#include <map>
#include <experimental/filesystem>
#include <locale>

#define STOP_MARK "#"
#define SLEEP_TIME 1000
#define FILE_STOP_MARK_1 ';'
#define FILE_STOP_MARK_2 '\n'
#define SKIP_N_CHARS 3
HANDLE hOut;
namespace fs = std::experimental::filesystem;

template<typename T>
void drawListOfColumnsToRead(const std::vector<T>& vec, int mode);
void initFilesForOne(std::fstream& in_file, std::fstream& out_file);
void getColumnsToRead(std::vector<int>& columns_to_read); 
void initColumns(std::vector<int>& columns_to_read);
std::map<int, std::string> generateMap(std::fstream& in_file, const std::vector<int>& selected); 
std::string findSelectedValues(std::fstream & in_file, const std::vector<int>& selected);
bool isInSelected(int index, const std::vector<int>& selected);
void setConsoleColor(HANDLE& hOut, int color = 0);
std::vector<std::string> getAllFilesNamesWithinFolder(std::string folder);
void removeExtensions(std::vector<std::string>& filenames);

void callFunctionSetForOneFile();
void callFunctionSetForAllFilesInDirSaveToOneFile(); 
void callFunctionSetForAllFilesInDirSaveSeparatedFiles();

int main()
{
	setlocale(LC_ALL, "polish");
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	int choice;
	
	setConsoleColor(hOut,3);
	std::cout << "----MENU----\n";
	setConsoleColor(hOut);

	std::cout<< "1.Operacja na jednym pliku\n"
		<< "2.Operacje na wszystkich plikach .csv w katalogu z zapisem do jednego pliku\n"
		<< "3.Operacje na wszystkich plikach .csv w katalogu z zapisem do odseparowanych plików\n"
		<< "4.Zamknij aplikacje\n"
		<< "wybor: ";
	
	std::cin >> choice;

	switch (choice) {
	case 1:
		callFunctionSetForOneFile();
		break;
	case 2:
		callFunctionSetForAllFilesInDirSaveToOneFile();
		break;
	case 3:
		callFunctionSetForAllFilesInDirSaveSeparatedFiles();
		break;
	case 4:
		return 0;
	default:
		std::cout << "Coœ posz³o nie tak w g³ównej funkcji programu\n";
		break;
	}
	
	std::cin.get();
	std::cin.get();

}
void initFilesForOne(std::fstream& in_file, std::fstream& out_file)
{
	bool should_rewrite_filename = true;
	std::string filename;

	while (should_rewrite_filename) {
		system("cls");
		std::cout << "Podaj nazwe pliku .csv(bez rozszerzenia, pamietaj aby program byl w tym samym folderze co plik)\nnazwa pliku: ";
		std::cin >> filename;
		filename += ".csv";
		in_file.open(filename, std::ios::in);

		if (in_file.good()) {
			should_rewrite_filename = false;
			setConsoleColor(hOut, 2);
			std::cout << "Udalo sie otworzyc plik!\n";
			setConsoleColor(hOut);
			Sleep(SLEEP_TIME);
		}
		else {
			setConsoleColor(hOut, 1);
			std::cout << "Nie udalo sie otworzyc pliku\n";
			Sleep(SLEEP_TIME);
			setConsoleColor(hOut);
		}
	}

	should_rewrite_filename = true;
	while (should_rewrite_filename) {
		system("cls");
		std::cout << "Podaj nazwe pilku do ktorego maja zostac zapisane dane(bez rozszerzenia)\nnazwa: ";
		std::cin >> filename;
		filename += ".txt";
		out_file.open(filename, std::ios::out | std::ios::app);
		if (out_file.good()) {
			setConsoleColor(hOut, 2);
			std::cout << "Udalo sie utworzyc nowy plik/odnalezc plik o podanej nazwie !";
			should_rewrite_filename = false;
			Sleep(SLEEP_TIME);
			setConsoleColor(hOut);
		}
		else {
			setConsoleColor(hOut, 1);
			std::cout << "Nie udalo sie utworzyc nowy plik/odnalezc plik o podanej nazwie\n";
			setConsoleColor(hOut);
			std::cout << "Jesli problem sie powtarza sprobuj otworzyc aplikacje w trybie administratora\n";
			Sleep(SLEEP_TIME);
		}
	}
}
void getColumnsToRead(std::vector<int>& columns_to_read)
{
	int column_number;
	std::string buffer;
	while (buffer != STOP_MARK)
	{
		std::cin.clear();
		std::cin >> buffer;
		std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::toupper);
		column_number = int(buffer[0]) - 65;
		if (column_number < 26 && column_number >= 0 && buffer.size() == 1) {
			columns_to_read.push_back(column_number);
		}
	}
}
void initColumns(std::vector<int>& columns_to_read)
{
	bool should_rewrite_column_list = true;

	while (should_rewrite_column_list) {
		columns_to_read.clear();
		system("cls");
		std::cout << "Podaj litery oznaczajace kolumny które chcesz przepisaæ \"" << STOP_MARK << "\" koñczy wpisywanie\n";
		std::cout << "kolumny: ";
		getColumnsToRead(columns_to_read);

		std::cout << "\nKolumny które zostan¹ przepisane: ";
		drawListOfColumnsToRead(columns_to_read, 1);

		std::cout << "\nCzy wszystko siê zgadza ? \n0.Mo¿emy zaczynaæ \n1.Chcê wpisaæ kolumny od nowa \nwybor: ";
		std::cin >> should_rewrite_column_list;
	}
}
std::map<int, std::string> generateMap(std::fstream& in_file, const std::vector<int>& selected)
{
	int row_it = 0;
	std::map<int, std::string> column_titles;
	std::string buffer;
	char k = ' ';
	
	in_file.clear();
	in_file.seekg(SKIP_N_CHARS, std::ios::beg);
	
	while (k != FILE_STOP_MARK_2) {
		k = in_file.get();
		if (k != FILE_STOP_MARK_1 && k != FILE_STOP_MARK_2) {
			buffer += k;
		}
		if (k == FILE_STOP_MARK_1) {
			if (isInSelected(row_it, selected)) {
				column_titles[row_it] += buffer + ": ";
			}
			row_it++;
			buffer = "";
		}
		else if (k == FILE_STOP_MARK_2) {
			if (isInSelected(row_it, selected)) {
				column_titles[row_it] += buffer + ": ";
				row_it++;
			}
			row_it = 0;
			buffer = "";
		}
	}
	return column_titles;
}
bool isInSelected(int index, const std::vector<int>& selected)
{
	for (int i = 0; i < selected.size(); i++) {
		if (index == selected[i]) {
			return true;
		}
	}
	return false;
}
void setConsoleColor(HANDLE& hOut, int color)
{
	switch (color) {
	case 0:
		SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
		break;
	case 1:
		SetConsoleTextAttribute(hOut, FOREGROUND_RED);
		break;
	case 2:
		SetConsoleTextAttribute(hOut, FOREGROUND_GREEN);
		break;
	case 3:
		SetConsoleTextAttribute(hOut, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		break;
	default:
		SetConsoleTextAttribute(hOut, FOREGROUND_RED);
		std::cout << "Coœ posz³o nie tak w funkcji ustawiaj¹cej kolor tekstu w konsoli, prawdopodobnie programista poda³ z³y argument\n";
		SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}
}
std::vector<std::string> getAllFilesNamesWithinFolder(std::string folder)
{
	std::vector<std::string> names;
	std::string search_path = folder + "/*.csv*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}
void removeExtensions(std::vector<std::string>& filenames)
{
	int extension_size = 4;
	for (int i = 0; i < filenames.size(); i++) {
		filenames[i] = filenames[i].substr(0, filenames[i].length() - extension_size);
	}
}
void callFunctionSetForOneFile()
{
	std::vector<int> columns_to_read;
	std::fstream in_file;
	std::fstream out_file;
	std::string filtered_data;
	initFilesForOne(in_file, out_file);
	initColumns(columns_to_read);
	filtered_data = findSelectedValues(in_file, columns_to_read);

	//std::cout << filtered_data;
	out_file << filtered_data;
	in_file.close();
	out_file.close();
	setConsoleColor(hOut, 2);
	std::cout << "\nPrawdopodobnie wszystko siê uda³o ;)" << std::endl;
	setConsoleColor(hOut);
	std::cout << "Wciœnij ENTER aby zakoñczyæ";
}
void callFunctionSetForAllFilesInDirSaveToOneFile()
{
	
	fs::path folder_location = fs::current_path();
	
	std::string str_folder_location = folder_location.string();
	std::string out_file_name;
	std::string filtered_data;
	
	std::vector<int> columns_to_read;
	std::vector<std::string> filenames = getAllFilesNamesWithinFolder(str_folder_location);
	std::fstream out_file;
	std::fstream in_file;

	bool should_change_filename = true;
	while (should_change_filename) {
		std::cout << "Podaj nazwê pliku wyjœciowego(bez rozszerzenia): ";
		std::cin >> out_file_name;
		out_file_name += ".txt";
		out_file.open(out_file_name, std::ios::app);
		if (out_file.good()) {
			setConsoleColor(hOut, 2);
			std::cout << "Uda³o siê utworzyæ nowy plik/odnaleŸæ plik o podanej nazwie !";
			should_change_filename = false;
			Sleep(SLEEP_TIME);
			setConsoleColor(hOut);
		}
		else {
			setConsoleColor(hOut, 1);
			std::cout << "Nie Uda³o siê utworzyæ/odnaleŸæ pliku o podanej nazwie" << std::endl;
			setConsoleColor(hOut);
			std::wcout<<"spróbuj jeszcze raz\n";
			Sleep(SLEEP_TIME);
		}

	}
	initColumns(columns_to_read);
	 
	for (int i = 0; i < filenames.size(); i++) {
		in_file.open(filenames[i], std::ios::in);
		if (in_file.good()) {
			filtered_data = findSelectedValues(in_file, columns_to_read);
			out_file << "--------Dane Z Pliku \"" << filenames[i] << "\"--------\n";
			std::cout << std::endl << filenames[i];
			setConsoleColor(hOut, 2);
			std::cout << " - otwarty!\n";
			setConsoleColor(hOut);
			out_file << filtered_data;
		}
		else {
			std::cout << filenames[i];
			setConsoleColor(hOut, 1);
			std::cout << " - wyst¹pi³ b³¹d\n";
			setConsoleColor(hOut);
		}
		in_file.close();
	}

	setConsoleColor(hOut, 2);
	std::cout << "\nPrawdopodobnie wszystko siê uda³o ;)" << std::endl;
	setConsoleColor(hOut);
	std::cout << "Wciœnij ENTER aby zakoñczyæ";
	out_file.close();
}
void callFunctionSetForAllFilesInDirSaveSeparatedFiles()
{

	fs::path folder_location = fs::current_path();

	std::string str_folder_location = folder_location.string();
	std::string filtered_data;

	std::vector<int> columns_to_read;
	std::vector<std::string> filenames = getAllFilesNamesWithinFolder(str_folder_location);
	std::fstream out_file;
	std::fstream in_file;

	removeExtensions(filenames);
	initColumns(columns_to_read);
	

	for (int i = 0; i < filenames.size(); i++) {
		out_file.open("przefiltrowany_" + filenames[i] + ".txt", std::ios::out);
		in_file.open(filenames[i] + ".csv", std::ios::in);
		if (in_file.good()) {
			filtered_data = findSelectedValues(in_file, columns_to_read);
			std::cout << std::endl << filenames[i];
			setConsoleColor(hOut, 2);
			std::cout << " - otwarty!\n";
			setConsoleColor(hOut);
			out_file << filtered_data;
		}
		else {
			std::cout << filenames[i];
			setConsoleColor(hOut, 1);
			std::cout << " - wyst¹pi³ b³¹d\n";
			setConsoleColor(hOut);
		}
		in_file.close();
		out_file.close();
	}

	setConsoleColor(hOut, 2);
	std::cout << "\nPrawdopodobnie wszystko siê uda³o ;)" << std::endl;
	setConsoleColor(hOut);
	std::cout << "Wciœnij ENTER aby zakoñczyæ";
	out_file.close();
}
std::string findSelectedValues(std::fstream & in_file, const std::vector<int>& selected)
{
	std::map<int, std::string> column_titles = generateMap(in_file, selected);
	std::string buffer = "";
	std::string filtered_data = "";
	int row_it = 0;
	int column_it = 0;
	char k;

	in_file.clear();
	in_file.seekg(SKIP_N_CHARS, std::ios::beg);
	
	while (!in_file.eof()) {
		
		k = in_file.get();
		if (k != FILE_STOP_MARK_1 && k != FILE_STOP_MARK_2) {
			buffer += k;
		}
		if (k == FILE_STOP_MARK_1) {
			if (isInSelected(row_it, selected) && column_it != 0) {
				filtered_data += column_titles[row_it] + buffer + "\n";
			}
			row_it++;
			buffer = "";
		}
		else if (k == FILE_STOP_MARK_2) {
			if (isInSelected(row_it, selected) && column_it != 0) {
				filtered_data += column_titles[row_it] + buffer + "\n";
				row_it++;
			}
			row_it = 0;
			buffer = "";
			if (column_it != 0)
				filtered_data += "\n";
			column_it++;
		}
		
	}
	return filtered_data;
}
template<typename T>
void drawListOfColumnsToRead(const std::vector<T>& vec, int mode)
{
	if (mode == 0) {
		for (int i = 0; i < vec.size(); i++) {
			std::cout << vec[i] << " ";
		}
	}
	else if (mode == 1) {
		setConsoleColor(hOut, 3);
		for (int i = 0; i < vec.size(); i++) {
			std::cout << char(vec[i] + 65) << " ";
		}
		setConsoleColor(hOut);
	}
}
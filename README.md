![cmake workflow](https://github.com/Volodya-Petrov/send-file-to-server-application/actions/workflows/cmake.yml/badge.svg)

# send-file-to-server-application

Клиент-сервер приложение для передачи файлов по сети. Сервер принимает файл от клиента и сохраняет его на диске. Клиент отправляет серверу файл.

У сервера можно указать порт, который слушать, и папку, куда сохранять файлы. У клиента можно указать адресс сервера, порт, файл, который нужно передать, и имя файла, с которым он сохранится на сервере.

## Запуск приложения
build projects using cmake
```bash
$ ./build
```
Перейдите в папку сбилженного проекта
```bash
$ cd build
```
#Server
```bash
$ cd Server
$ ./Server port directory
```
#Client
```bash
$ cd Client
$ ./Server ip port path_to_file file_name_for_server
```

from src import rhodesia
import sys

if __name__ == '__main__':
    while True:
        try:
            command = input("rhodesia > ")
            result, error = rhodesia.start('<stdin>', command)
            if error: print(error.error_string())
            else: print(result)
        except KeyboardInterrupt:
            print("Bye!")
            break

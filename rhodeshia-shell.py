import rhodesia
import sys, os

if __name__ == '__main__':
    while True:
        try:
            command = input("rhodesia > ")
            result, error = rhodesia.start('<stdin>', command)
            if error: print(error.error_string())
            else: print(result)
            
            if command == 'cls':
                os.system('cls')
        except KeyboardInterrupt:
            print("Bye!")
            break

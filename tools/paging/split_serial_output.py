import os

if __name__ == "__main__":
    INPUT_FILE: str = os.path.join(os.path.dirname(__file__), "../../serial_output")
    RESULT_FILE_1: str = os.path.join(os.path.dirname(__file__), "pt_dump_1.txt")
    RESULT_FILE_2: str = os.path.join(os.path.dirname(__file__), "pt_dump_2.txt")

    state: int = 0

    with open(INPUT_FILE, "r") as fd_input:
        fd_output = None
        for line in fd_input:
            if "START PAGE TABLE DUMP 1" in line:
                fd_output = open(RESULT_FILE_1, "w")
                state = 1
                continue
            if "END PAGE TABLE DUMP 1" in line:
                fd_output.close()
                state = 0

            if "START PAGE TABLE DUMP 2" in line:
                fd_output = open(RESULT_FILE_2, "w")
                state = 1
                continue
            if "END PAGE TABLE DUMP 2" in line:
                fd_output.close()
                break

            if state == 1:
                fd_output.write(line)
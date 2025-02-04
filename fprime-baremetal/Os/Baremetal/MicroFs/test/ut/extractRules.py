import re


def extract_rules(file_name):
    with open(file_name, "r") as file:
        content = file.read()

    pattern = r"struct\s+(\w+)\s+: public STest::Rule<Os::Tester>"
    matches = re.findall(pattern, content)

    for match in matches:
        print(match)


# Replace 'filename.txt' with the path to your file
extract_rules("MyRules.hpp")

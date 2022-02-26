print("hello")

clean_words = []
with open("resources/word_list_1.txt", "r") as f:
    lines = f.readlines()
    for line in lines :
        # line = line.strip('\n')
        line = line.replace(' ', '')
        line = line.lower()
        print(line)
        clean_words.append(line)
    pass

with open("resources/clean_words.txt", "w") as f:
    f.writelines(clean_words)
    pass
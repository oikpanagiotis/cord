# Write tool to generate basic struct and function definitions for every object in https://discord.com/developers/docs/resources/channel

OPEN_BRACKET = '{'
CLOSE_BRACKET = '}'

def get_raw_names():
    names = []
    with open('discord_objects_names.txt') as file:
        for line in file.readlines():
            names.append(line.strip())
    return names

def get_struct_names():
    names = []
    with open('discord_objects_names.txt') as file:
        for line in file.readlines():
            words = list(map(lambda x: x.lower(), line.strip().split()))
            struct_name = '_'.join(words)
            names.append(f'cord_{struct_name}_t')
    return names

# raw_names = get_raw_names()
# struct_names = get_struct_names()

for name in get_raw_names():
    print(name)

# for name in get_struct_names():
    # print(f'typedef struct {name} {OPEN_BRACKET}')
    # print(f'{CLOSE_BRACKET}')
    # print('')

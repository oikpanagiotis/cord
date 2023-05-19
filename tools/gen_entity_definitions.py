# Write tool to generate basic struct and function definitions for every object in https://discord.com/developers/docs/resources/channel

OPEN_BRACKET = '{'
CLOSE_BRACKET = '}'

def print_struct_start(name):
    print(f'typedef struct {name} {OPEN_BRACKET}')

def print_struct_field(type, name):
    print(f'    {type} {name};')

def print_struct_end(name):
    print(f'{CLOSE_BRACKET} {name};')


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


# Create actual struct value
# Parse names/fields from text file and serialize them into struct objects(name, properties[])
# Print all of the structs

for name in get_raw_names():
    print(name)

for name in get_struct_names():
    print_struct_start(name)
    print_struct_field('int', 'user_id')
    print_struct_end(name)
    print('')

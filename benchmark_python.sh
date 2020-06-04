(cd enclave/python/build; ./python -m pip install html5lib chameleon pyaes)
echo "\nhtml5lin"
./python -m timeit "import os
import html5lib

file1 = open('./tests/h2o/examples/doc_root/index.html','rb')
html5lib.parse(file1)"

echo ""
echo "pyaes"

./python -m timeit "from six.moves import xrange
import pyaes

CLEARTEXT = b'This is a test. What could possibly go wrong? ' * 500

KEY = b'\xa1\xf6%\x8c\x87}_\xcd\x89dHE8\xbf\xc9,'


def bench_pyaes(loops):
    range_it = xrange(loops)

    for loops in range_it:
        aes = pyaes.AESModeOfOperationCTR(KEY)
        ciphertext = aes.encrypt(CLEARTEXT)

        # need to reset IV for decryption
        aes = pyaes.AESModeOfOperationCTR(KEY)
        plaintext = aes.decrypt(ciphertext)

        # explicitly destroy the pyaes object
        aes = None

    if plaintext != CLEARTEXT:
        raise Exception('decrypt error!')

bench_pyaes(5)"

echo ""
echo  "json_load"

./python -m timeit "
import json
import random
import six
import sys

DICT = {
    'ads_flags': 0,
    'age': 18,
    'bulletin_count': 0,
    'comment_count': 0,
    'country': 'BR',
    'encrypted_id': 'G9urXXAJwjE',
    'favorite_count': 9,
    'first_name': '',
    'flags': 412317970704,
    'friend_count': 0,
    'gender': 'm',
    'gender_for_display': 'Male',
    'id': 302935349,
    'is_custom_profile_icon': 0,
    'last_name': '',
    'locale_preference': 'pt_BR',
    'member': 0,
    'tags': ['a', 'b', 'c', 'd', 'e', 'f', 'g'],
    'profile_foo_id': 827119638,
    'secure_encrypted_id': 'Z_xxx2dYx3t4YAdnmfgyKw',
    'session_number': 2,
    'signup_id': '201-19225-223',
    'status': 'A',
    'theme': 1,
    'time_created': 1225237014,
    'time_updated': 1233134493,
    'unread_message_count': 0,
    'user_group': '0',
    'username': 'collinwinter',
    'play_count': 9,
    'view_count': 7,
    'zip': ''}

TUPLE = (
    [265867233, 265868503, 265252341, 265243910, 265879514,
     266219766, 266021701, 265843726, 265592821, 265246784,
     265853180, 45526486, 265463699, 265848143, 265863062,
     265392591, 265877490, 265823665, 265828884, 265753032], 60)

def mutate_dict(orig_dict, random_source):
    new_dict = dict(orig_dict)
    for key, value in new_dict.items():
        rand_val = random_source.random() * sys.maxsize
        if isinstance(key, six.integer_types + (bytes, six.text_type)):
            new_dict[key] = type(key)(rand_val)
    return new_dict


random_source = random.Random(5)  # Fixed seed.
DICT_GROUP = [mutate_dict(DICT, random_source) for _ in range(3)]


def bench_json_loads(objs):
    for obj in objs:
        # 20 loads
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)
        json.loads(obj)


json_dict = json.dumps(DICT)
json_tuple = json.dumps(TUPLE)
json_dict_group = json.dumps(DICT_GROUP)
objs = (json_dict, json_tuple, json_dict_group)
"

echo ""
echo  "fannkuch"
./python -m timeit "from six.moves import xrange

DEFAULT_ARG = 9

def fannkuch(n):
    count = list(xrange(1, n + 1))
    max_flips = 0
    m = n - 1
    r = n
    check = 0
    perm1 = list(xrange(n))
    perm = list(xrange(n))
    perm1_ins = perm1.insert
    perm1_pop = perm1.pop

    while 1:
        if check < 30:
            check += 1

        while r != 1:
            count[r - 1] = r
            r -= 1

        if perm1[0] != 0 and perm1[m] != m:
            perm = perm1[:]
            flips_count = 0
            k = perm[0]
            while k:
                perm[:k + 1] = perm[k::-1]
                flips_count += 1
                k = perm[0]

            if flips_count > max_flips:
                max_flips = flips_count

        while r != n:
            perm1_ins(r, perm1_pop(0))
            count[r] -= 1
            if count[r] > 0:
                break
            r += 1
        else:
            return max_flips


arg = DEFAULT_ARG
fannkuch(arg)"


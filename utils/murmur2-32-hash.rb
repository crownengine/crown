# Copyright (c) 2013 Daniele Bartolini, Michele Rossi
# Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

# MurmurHash2, by Austin Appleby
def murmur2_32(string, seed)

    m = 0x5bd1e995
    r = 24
    len = string.length

    h = seed ^ len

    data = string.bytes.to_a

    while len >= 4
      	k = data[0]
      	k |= data[1] << 8
      	k |= data[2] << 16
      	k |= data[3] << 24

      	k = ( k * m ) % 0x100000000
      	k ^= k >> r
      	k = ( k * m ) % 0x100000000

      	h = ( h * m ) % 0x100000000
      	h ^= k

      	len -= 4
    end

    if len == 3 then
      h ^= data[-1] << 16
      h ^= data[-2] << 8
      h ^= data[-3]
    end
    if len == 2 then
      h ^= data[-1] << 8
      h ^= data[-2]
    end
    if len == 1 then
      h ^= data[-1]
    end

    h = ( h * m ) % 0x100000000
    h ^= h >> 13
    h = ( h * m ) % 0x100000000
    h ^= h >> 15

    return h
end

if ARGV.length != 2
	print "Usage: ruby murmur2-hash.rb <string> <seed>\n"
	exit
end

result = murmur2_32(ARGV[0], ARGV[1].to_i)

print result.to_s(16) + "\n";
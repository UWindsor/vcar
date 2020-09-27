import os

to_emb_fifo = "/tmp/rest-emb"
from_emb_fifo = "/tmp/emb-rest"

fd = open(to_emb_fifo, 'w')
fd.write("Request to embedded\n")
fd.close()

fd = open(from_emb_fifo, 'r')
res = fd.read()
print(res)
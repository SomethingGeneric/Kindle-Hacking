import sys,os

goal = sys.argv[1]

mkf = sys.argv[2]

with open(mkf) as f:
    makef = f.read()

new = makef.replace("/usr/local",goal,1)

os.remove(mkf)

with open(mkf,"w") as f:
    f.write(makef)
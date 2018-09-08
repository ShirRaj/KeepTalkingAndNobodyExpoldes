import Actions
from IModule import IModule
import random
import zope.event

optional_settings = {0: ['sc', 'sc', 'sc'], 1: ['W', 'W', 'cnW', 'anW'], 2: ['Y', 'Y', 'cnY', 'anY'],
                     3: ['sc', 'sc', 'sc', 'dc', 'dc'], 4: ['R', 'R', 'cnR', 'cnR', 'cnR'],
                     5: ['G', 'G', 'cnG', 'cnG', 'cnG'], 6: ['sc', 'sc', 'sc'], 7: ['W', 'W', 'cnW', 'anW'],
                     8: ['Y', 'Y', 'cY', 'anY'], 9: ['sc', 'sc', 'sc', 'dc', 'dc'], 10: ['R', 'R', 'cnR', 'cnR', 'cnR'],
                     11: ['G', 'G', 'cnG', 'cnG', 'cnG']}

all_colors = ['Y', 'R', 'W', 'B', 'G']

"""
sc - same color
cnX - any color not empty, not X
anX - any color not X
dc - different color
"""


class ModuleWires(IModule):
    def data(self):
        global optional_settings, all_colors
        if self.bomb == "Dynamite":
            setup_id = random.randint(6, 11)
            setup = optional_settings[setup_id]
        else:
            setup_id = random.randint(0, 5)
            setup = optional_settings[setup_id]
        chosen = self.get_colors(setup)
        order = random.shuffle(chosen)
        self.send_msg_to_arduino(Actions.DATA, 'ROW_DATA', ','.join(order))
        self.send_msg_to_arduino(Actions.DATA, 'SETUP_DATA', setup_id)
        zope.event.notify("HOST WIRES COLORS:" + ','.join(order))


    def get_colors(self, setup):
        chosen = []
        for i in setup:
            if i == 'sc' and len(chosen) == 0:
                chosen.append(random.randint(0, 4))
            elif i == 'sc':
                chosen.append(chosen[len(chosen) - 1])
            elif 'cn' in i:
                forbidden = i[2]
                location = all_colors.index(forbidden)
                r = range(0, 4)
                r.remove(location)
                chosen.append(all_colors[random.choice(r)])
            elif 'an' in i:
                forbidden = i[2]
                location = all_colors.index(forbidden)
                r = range(0, 5)
                r.remove(location)
                j = random.choice(r)
                if j == 5:
                    chosen.append('E')
                else:
                    chosen.append(all_colors[j])
            elif 'dc' in i:
                to_remove = []
                for j in chosen:
                    to_remove.append(all_colors.index(j))
                to_remove = list(set(to_remove))
                r = range(0, 4)
                for j in to_remove:
                    r.remove(j)
                chosen.append(all_colors[random.choice(r)])
            while len(chosen) < 5:
                chosen.append('E')
            return chosen

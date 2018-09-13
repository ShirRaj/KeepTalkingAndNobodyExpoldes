import Const
from Manager import Manager
import zope.event

manager = None
ready_count = 0


def handle_event(event):
    if event.startswith('HOST'):
        action = event.split(' ')[1]
        if action == "READY" or action == "UNREADY":
            handle_ready_event(event)
        elif action == "BOOM":
            handle_boom_event(event)
        elif action == "SOLVED":
            handle_solved_event(event)


def handle_ready_event(event):
    global ready_count
    if event.split(' ')[1] == "READY":
        ready_count += 1
    elif event.split(' ')[1] == "UNREADY":
        ready_count -= 1

    if ready_count == Const.MODULE_COUNT:
        print "ready"
        manager.start_game()


def handle_boom_event(event):
    manager.stop_game()


def handle_solved_event(event):
    manager.stop_game()


if __name__ == "__main__":
    # global manager
    manager = Manager()
    manager.set_modules_and_order()

    print "Selecte modules are:" + str(manager.modules_order)
    print "Waiting for ready state from modules"

    zope.event.subscribers.append(handle_event)

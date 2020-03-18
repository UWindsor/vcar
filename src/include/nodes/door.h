int door_locked = 1;

void node_door_mode()
{
  Log("Door mode detected ...");
}

void node_door_unlock()
{
  Log("Unlocking the door ...");
  door_locked = 0;
  Log("Door unlocked ...");
}

void node_door_lock()
{
  Log("Locking the door ...");
  door_locked = 1;
  Log("Door locked ...");
}

void node_door_mirror()
{
  Log("Mirror signal received ...");
}

#pragma once

template<typename EventListener, int MAX_LISTENERS = 2>

class Event {
public:
  Event()
    : listenerCount(0) {}

  bool addListener(EventListener *l) {
    if (listenerCount >= MAX_LISTENERS)
      return false;  // Kein Platz mehr
    listeners[listenerCount++] = l;
    return true;
  }

  bool removeListener(EventListener *l) {
    for (int i = 0; i < listenerCount; i++) {
      if (listeners[i] == l) {
        // Listener gefunden, verschiebe alle nachfolgenden nach links
        for (int j = i; j < listenerCount - 1; j++) {
          listeners[j] = listeners[j + 1];
        }
        listenerCount--;
        listeners[listenerCount] = nullptr;  // Optional: letzten Eintrag nullen
        return true;
      }
    }
    return false;  // Listener nicht gefunden
  }

protected:
  EventListener *listeners[MAX_LISTENERS] = { nullptr };
  int listenerCount;
};
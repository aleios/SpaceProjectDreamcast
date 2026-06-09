from PyQt6.QtWidgets import QApplication

class TabsFinderMixin:
    def find_main_window(self):
        for widget in QApplication.topLevelWidgets():
            if hasattr(widget, 'tabsPages'):
                return widget
        return None
    def navigate_to_tab(self, tab_class, callback=None):
        main_window = self.find_main_window()
        if main_window:
            for i in range(main_window.tabsPages.count()):
                widget = main_window.tabsPages.widget(i)
                if isinstance(widget, tab_class):
                    main_window.tabsPages.setCurrentIndex(i)
                    if callback:
                        callback(widget)
                    break
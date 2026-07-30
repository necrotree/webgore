#!/usr/bin/env python3
"""Webgore — a WebKit browser for Windows."""

import sys

import gi

gi.require_version('Gtk', '3.0')
gi.require_version('WebKit2', '4.0')

from gi.repository import Gtk

from webgore.browser import BrowserWindow


def main():
    window = BrowserWindow()
    window.connect('destroy', Gtk.main_quit)
    window.show_all()
    Gtk.main()


if __name__ == '__main__':
    sys.exit(main())

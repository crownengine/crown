/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_OSX

#include "device.h"
#include "memory.h"
#include "os_event_queue.h"
#include "os_window_osx.h"
#include "thread.h"
#include "main.h"
#include "command_line.h"
#include "disk_filesystem.h"
#include "crown.h"
#include "bundle_compiler.h"
#include "console_server.h"
#include <Cocoa/Cocoa.h>
#include <bgfxplatform.h>
#include <bgfx.h>

 @interface AppDelegate : NSObject<NSApplicationDelegate>
{
	bool terminated;
}

+ (AppDelegate *)sharedDelegate;
- (id)init;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (bool)applicationHasTerminated;

@end

@interface Window : NSObject<NSWindowDelegate>
{
	uint32_t windowCount;
}

+ (Window*)sharedDelegate;
- (id)init;
- (void)windowCreated:(NSWindow*)window;
- (void)windowWillClose:(NSNotification*)notification;
- (BOOL)windowShouldClose:(NSWindow*)window;
- (void)windowDidResize:(NSNotification*)notification;

@end

namespace crown
{

static bool s_exit = false;
static uint8_t s_translateKey[256];

struct MainThreadArgs
{
	Filesystem* fs;
	ConfigSettings* cs;
};	

int32_t func(void* data)
{
	MainThreadArgs* args = (MainThreadArgs*) data;
	if(!crown::init(*args->fs, *args->cs))
	{
		return EXIT_FAILURE;
	}
	crown::update();
	crown::shutdown();

	return EXIT_SUCCESS;
}

struct OsxDevice
{
	OsxDevice()
		: _ns_window(NULL)
	{
		s_translateKey[27]             = KeyboardButton::ESCAPE;
		s_translateKey[13]             = KeyboardButton::ENTER;
		s_translateKey[9]              = KeyboardButton::TAB;
		s_translateKey[127]            = KeyboardButton::BACKSPACE;
		s_translateKey[uint8_t(' ')]   = KeyboardButton::SPACE;

		s_translateKey[uint8_t('0')]   = KeyboardButton::NUM_0;
		s_translateKey[uint8_t('1')]   = KeyboardButton::NUM_1;
		s_translateKey[uint8_t('2')]   = KeyboardButton::NUM_2;
		s_translateKey[uint8_t('3')]   = KeyboardButton::NUM_3;
		s_translateKey[uint8_t('4')]   = KeyboardButton::NUM_4;
		s_translateKey[uint8_t('5')]   = KeyboardButton::NUM_5;
		s_translateKey[uint8_t('6')]   = KeyboardButton::NUM_6;
		s_translateKey[uint8_t('7')]   = KeyboardButton::NUM_7;
		s_translateKey[uint8_t('8')]   = KeyboardButton::NUM_8;
		s_translateKey[uint8_t('9')]   = KeyboardButton::NUM_9;

		for (char ch = 'a'; ch <= 'z'; ++ch)
		{
			s_translateKey[uint8_t(ch)]             =
			s_translateKey[uint8_t(ch - ' ')] = KeyboardButton::A + (ch - 'a');
		}
	}

	NSEvent* waitEvent()
	{
		return [NSApp
			nextEventMatchingMask:NSAnyEventMask
			untilDate:[NSDate distantFuture] // wait for event
			inMode:NSDefaultRunLoopMode
			dequeue:YES
			];
	}

	NSEvent* peekEvent()
	{
		return [NSApp
			nextEventMatchingMask:NSAnyEventMask
			untilDate:[NSDate distantPast] // do not wait for event
			inMode:NSDefaultRunLoopMode
			dequeue:YES
			];
	}

	void getMousePos(int* outX, int* outY)
	{
		NSWindow* window = _ns_window;
		NSRect originalFrame = [window frame];
		NSPoint location = [window mouseLocationOutsideOfEventStream];
		NSRect adjustFrame = [NSWindow contentRectForFrameRect: originalFrame styleMask: NSTitledWindowMask];

		int x = location.x;
		int y = (int)adjustFrame.size.height - (int)location.y;

		// clamp within the range of the window

		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x > (int)adjustFrame.size.width) x = (int)adjustFrame.size.width;
		if (y > (int)adjustFrame.size.height) y = (int)adjustFrame.size.height;

		*outX = x;
		*outY = y;
	}

	uint8_t translateModifiers(int flags)
	{
		uint8_t mask = 0;

		if (flags & NSShiftKeyMask)
			mask |= ModifierButton::SHIFT;

		if (flags & NSAlternateKeyMask)
			mask |= ModifierButton::ALT;

		if (flags & NSControlKeyMask)
			mask |= ModifierButton::CTRL;

		return mask;
	}

	KeyboardButton::Enum handleKeyEvent(NSEvent* event, uint8_t* specialKeys)
	{
		NSString* key = [event charactersIgnoringModifiers];
		unichar keyChar = 0;

		if ([key length] == 0)
		{
			return KeyboardButton::NONE;
		}

		keyChar = [key characterAtIndex:0];

		int keyCode = keyChar;
		*specialKeys = translateModifiers([event modifierFlags]);

		// if this is a unhandled key just return None
		if (keyCode < 256)
		{
			return (KeyboardButton::Enum)s_translateKey[keyCode];
		}

		switch (keyCode)
		{
		case NSF1FunctionKey:  return KeyboardButton::F1;
		case NSF2FunctionKey:  return KeyboardButton::F2;
		case NSF3FunctionKey:  return KeyboardButton::F3;
		case NSF4FunctionKey:  return KeyboardButton::F4;
		case NSF5FunctionKey:  return KeyboardButton::F5;
		case NSF6FunctionKey:  return KeyboardButton::F6;
		case NSF7FunctionKey:  return KeyboardButton::F7;
		case NSF8FunctionKey:  return KeyboardButton::F8;
		case NSF9FunctionKey:  return KeyboardButton::F9;
		case NSF10FunctionKey: return KeyboardButton::F10;
		case NSF11FunctionKey: return KeyboardButton::F11;
		case NSF12FunctionKey: return KeyboardButton::F12;

		case NSLeftArrowFunctionKey:   return KeyboardButton::LEFT;
		case NSRightArrowFunctionKey:  return KeyboardButton::RIGHT;
		case NSUpArrowFunctionKey:     return KeyboardButton::UP;
		case NSDownArrowFunctionKey:   return KeyboardButton::DOWN;

		case NSPageUpFunctionKey:      return KeyboardButton::PAGE_UP;
		case NSPageDownFunctionKey:    return KeyboardButton::PAGE_DOWN;
		case NSHomeFunctionKey:        return KeyboardButton::HOME;
		}

		return KeyboardButton::NONE;
	}

	int32_t run(Filesystem* fs, ConfigSettings* cs)
	{
		[NSApplication sharedApplication];

		id dg = [AppDelegate sharedDelegate];
		[NSApp setDelegate:dg];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		[NSApp activateIgnoringOtherApps:YES];
		[NSApp finishLaunching];

		[[NSNotificationCenter defaultCenter]
				postNotificationName:NSApplicationWillFinishLaunchingNotification
				object:NSApp];

		[[NSNotificationCenter defaultCenter]
			postNotificationName:NSApplicationDidFinishLaunchingNotification
			object:NSApp];

		id quitMenuItem = [NSMenuItem new];
		[quitMenuItem
			initWithTitle:@"Quit"
			action:@selector(terminate:)
			keyEquivalent:@"q"];

		id appMenu = [NSMenu new];
		[appMenu addItem:quitMenuItem];

		id appMenuItem = [NSMenuItem new];
		[appMenuItem setSubmenu:appMenu];

		id menubar = [[NSMenu new] autorelease];
		[menubar addItem:appMenuItem];
		[NSApp setMainMenu:menubar];

		NSRect rect = NSMakeRect(0, 0, cs->window_width, cs->window_height);
		NSWindow* window = [[NSWindow alloc]
			initWithContentRect:rect
			styleMask:0
			|NSTitledWindowMask
			|NSClosableWindowMask
			|NSMiniaturizableWindowMask
			|NSResizableWindowMask
			backing:NSBackingStoreBuffered defer:NO
		];

		CE_ASSERT(window != NULL, "Unable to create native window");

		[window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
		[window makeKeyAndOrderFront:window];
		[window setAcceptsMouseMovedEvents:YES];
		[window setBackgroundColor:[NSColor blackColor]];
		[[Window sharedDelegate] windowCreated:window];

		_ns_window = window;

		oswindow_set_window(_ns_window);
		bgfx::osxSetNSWindow(_ns_window);

		// Start main thread
		MainThreadArgs mta;
		mta.fs = fs;
		mta.cs = cs;

		Thread main_thread;
		main_thread.start(func, &mta);

		_queue.push_metrics_event(0, 0, cs->window_width, cs->window_height);

		while (!(s_exit = [dg applicationHasTerminated]) )
		{
			if (bgfx::RenderFrame::Exiting == bgfx::renderFrame() )
			{
				break;
			}

			while (dispatchEvent(peekEvent() ) )
			{
			}
		}

		while (bgfx::RenderFrame::NoContext != bgfx::renderFrame() ) {};
		main_thread.stop();

		return EXIT_SUCCESS;
	}

	void windowDidResize()
	{
		NSWindow* window = _ns_window;
		NSRect rect = [window frame];
		uint32_t width  = uint32_t(rect.size.width);
		uint32_t height = uint32_t(rect.size.height);
		_queue.push_metrics_event(0, 0, width, height);
	}

	bool dispatchEvent(NSEvent* event)
	{
		if (event)
		{
			NSEventType eventType = [event type];

			switch (eventType)
			{
				case NSMouseMoved:
				{
					int x, y;
					getMousePos(&x, &y);
					_queue.push_mouse_event(x, y);
					break;
				}

				case NSLeftMouseDown:
				{
					int x, y;
					getMousePos(&x, &y);
					_queue.push_mouse_event(x, y, MouseButton::LEFT, true);
					break;
				}

				case NSLeftMouseUp:
				{
					int x, y;
					getMousePos(&x, &y);
					_queue.push_mouse_event(x, y, MouseButton::LEFT, false);
					break;
				}

				case NSRightMouseDown:
				{
					int x, y;
					getMousePos(&x, &y);
					_queue.push_mouse_event(x, y, MouseButton::RIGHT, true);
					break;
				}

				case NSRightMouseUp:
				{
					int x, y;
					getMousePos(&x, &y);
					_queue.push_mouse_event(x, y, MouseButton::RIGHT, false);
					break;
				}

				case NSKeyDown:
				{
					uint8_t modifiers = 0;
					KeyboardButton::Enum key = handleKeyEvent(event, &modifiers);

					// If KeyCode is none we don't don't handle the key
					// Note that return false here means that we take care of the key (instead of the default behavior)
					if (key != KeyboardButton::NONE)
					{
						_queue.push_keyboard_event(modifiers, key, true);
						return false;
					}

					break;
				}

				case NSKeyUp:
				{
					uint8_t modifiers  = 0;
					KeyboardButton::Enum key = handleKeyEvent(event, &modifiers);

					if (key != KeyboardButton::NONE)
					{
						_queue.push_keyboard_event(modifiers, key, false);
						return false;
					}

					break;
				}
			}

			[NSApp sendEvent:event];
			[NSApp updateWindows];

			return true;
		}

		return false;
	}

public:

	OsEventQueue _queue;
	NSWindow* _ns_window;
};

static OsxDevice s_odvc;	

bool next_event(OsEvent& ev)
{
	return s_odvc._queue.pop_event(ev);
}

} // namespace crown

@implementation AppDelegate

+ (AppDelegate *)sharedDelegate
{
	static id delegate = [AppDelegate new];
	return delegate;
}

- (id)init
{
	self = [super init];

	if (nil == self)
	{
		return nil;
	}

	self->terminated = false;
	return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	CE_UNUSED(sender);
	self->terminated = true;
	return NSTerminateCancel;
}

- (bool)applicationHasTerminated
{
	return self->terminated;
}

@end

@implementation Window

+ (Window*)sharedDelegate
{
	static id windowDelegate = [Window new];
	return windowDelegate;
}

- (id)init
{
	self = [super init];
	if (nil == self)
	{
		return nil;
	}

	self->windowCount = 0;
	return self;
}

- (void)windowCreated:(NSWindow*)window
{
	assert(window);

	[window setDelegate:self];

	assert(self->windowCount < ~0u);
	self->windowCount += 1;
}

- (void)windowWillClose:(NSNotification*)notification
{
	CE_UNUSED(notification);
}

- (BOOL)windowShouldClose:(NSWindow*)window
{
	assert(window);

	[window setDelegate:nil];

	assert(self->windowCount);
	self->windowCount -= 1;

	if (self->windowCount == 0)
	{
		[NSApp terminate:self];
		return false;
	}

	return true;
}

- (void)windowDidResize:(NSNotification*)notification
{
	CE_UNUSED(notification);
	using namespace crown;
	s_odvc.windowDidResize();
}
@end

int main(int argc, char** argv)
{
	using namespace crown;

	ConfigSettings cs;
	parse_command_line(argc, argv, cs);

	memory_globals::init();
	{
		DiskFilesystem fs(cs.source_dir);
		parse_config_file(fs, cs);
	}

	console_server_globals::init(cs.console_port, cs.wait_console);

	bundle_compiler_globals::init(cs.source_dir, cs.bundle_dir);

	bool do_continue = true;
	int exitcode = EXIT_SUCCESS;

	do_continue = bundle_compiler::main(cs.do_compile, cs.do_continue, cs.platform);

	if (do_continue)
	{
		DiskFilesystem dst_fs(cs.bundle_dir);
		exitcode = crown::s_odvc.run(&dst_fs, &cs);
	}

	bundle_compiler_globals::shutdown();
	console_server_globals::shutdown();
	memory_globals::shutdown();
	return exitcode;
}

#endif // CROWN_PLATFORM_OSX
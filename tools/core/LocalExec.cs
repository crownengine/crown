/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;
using System;

namespace Crown
{
	public class LocalExec
	{
		public Process _process;

		public LocalExec()
		{
			_process = new Process();
			_process.EnableRaisingEvents = true;
		}

		public void Close()
		{
			if (!_process.HasExited)
			{
				_process.Kill();
				_process.Close();
			}
		}

		public void Start(string name, string args, string workdir)
		{
			ProcessStartInfo startInfo = new ProcessStartInfo();
			startInfo.FileName = name;
			startInfo.Arguments = args;
			startInfo.WorkingDirectory = workdir;

			_process.StartInfo = startInfo;
			_process.Start();
		}
	}
}

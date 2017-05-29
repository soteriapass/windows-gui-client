using System;

namespace PasswordManager
{
    class BusyScope 
    {
        #region Nested

        private class DispsableScope : IDisposable
        {
            BusyScope _Parent;

            public DispsableScope(BusyScope parent)
            {
                _Parent = parent;
                ++_Parent.BusyCount;
            }

            public void Dispose()
            {
                --_Parent.BusyCount;
            }
        }

        #endregion

        #region Variables

        private int _BusyCount;
        private readonly Action _OnStart;
        private readonly Action _OnEnd;

        #endregion

        #region Ctor

        public BusyScope(Action start, Action end)
        {
            _OnStart = start;
            _OnEnd = end;
        }

        #endregion

        #region Properties

        public int BusyCount
        {
            get { return _BusyCount; }
            set
            {
                if(_BusyCount != value)
                {
                    if (_BusyCount == 0 && value == 1)
                    {
                        _OnStart();
                    }
                    else if(_BusyCount == 1 && value == 0)
                    {
                        _OnEnd();
                    }
                    _BusyCount = value;
                }
            }
        }

        #endregion

        #region Methods

        public IDisposable Start()
        {
            return new DispsableScope(this);
        }

        #endregion
    }
}

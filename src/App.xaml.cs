using System.Windows;

namespace PasswordManager
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        #region Variables

        private static App _Instance;
        private Conf _Conf;

        #endregion

        #region Ctor

        public App()
        {
            _Instance = this;
            _Conf = Conf.Load("pswmgr.conf");
        }

        #endregion

        #region Properties

        public static App Instance
        {
            get { return _Instance; }
        }

        internal Conf Conf
        {
            get { return _Conf; }
        }

        #endregion
    }
}

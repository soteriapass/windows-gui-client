using HtmlAgilityPack;
using PasswordManager.Utilities;
using System;
using System.ComponentModel;
using System.IO;
using System.Net;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;

namespace PasswordManager.ObjectModel
{
    class PasswordWrapper : INotifyPropertyChanged
    {
        #region Variables

        private readonly Pswmgr.PasswordEntry _PasswordEntry;
        private bool _SearchedForImage;

        #endregion

        #region Ctor

        public PasswordWrapper(Pswmgr.PasswordEntry passwordEntry)
        {
            _PasswordEntry = passwordEntry;
        }

        #endregion

        #region Properties

        public string AccountName
        {
            get { return _PasswordEntry.AccountName; }
            set { _PasswordEntry.AccountName = value; }
        }

        public string Extra
        {
            get { return _PasswordEntry.Extra; }
            set { _PasswordEntry.Extra = value; }
        }

        public BitmapImage Icon
        {
            get { return GetIcon(); }
            set { }
        }

        public string Password
        {
            get { return _PasswordEntry.Password; }
            set { _PasswordEntry.Password = value; }
        }

        public string Username
        {
            get { return _PasswordEntry.Username; }
            set { _PasswordEntry.Username = value; }
        }

        public Pswmgr.PasswordEntry WrappedItem
        {
            get { return _PasswordEntry; }
        }

        #endregion

        #region Methods

        private BitmapImage GetIcon()
        {
            if (!_SearchedForImage)
            {
                string file = Path.Combine(Path.GetTempPath(), "soteriapass", Extra.Replace(':', '_').Replace('.', '_').Replace('/', '_'), "favicon.ico");
                string fileAlt = Path.ChangeExtension(file, ".sec.ico");
                Directory.CreateDirectory(Directory.GetParent(file).FullName);
                if (File.Exists(file))
                {
                    BitmapImage returnValue = ExceptionUtilities.TryAssignCatchIgnore(delegate { return new BitmapImage(new Uri(file, UriKind.RelativeOrAbsolute)); }, null);
                    if (returnValue != null)
                        return returnValue;
                }
                if (File.Exists(fileAlt))
                {

                }
                if (!_SearchedForImage)
                {
                    _SearchedForImage = true;
                    Task.Run(delegate
                    {
                        DownloadFile(Extra, file);
                        if (ExceptionUtilities.TryAssignCatchIgnore(delegate { return new BitmapImage(new Uri(file, UriKind.RelativeOrAbsolute)); }, null) != null)
                        {
                            OnPropertyChanged(nameof(Icon));
                            return;
                        }

                        DownloadFileAlternative(Extra, fileAlt);
                        if (ExceptionUtilities.TryAssignCatchIgnore(delegate { return new BitmapImage(new Uri(fileAlt, UriKind.RelativeOrAbsolute)); }, null) != null)
                        {
                            OnPropertyChanged(nameof(Icon));
                            return;
                        }
                    });
                }
            }
            return new BitmapImage(new Uri("/PasswordManager;component/Resources/shield.ico", UriKind.RelativeOrAbsolute));
        }

        private void DownloadFile(string value, string file)
        {
            using (WebClient client = new WebClient())
            {
                ExceptionUtilities.TryCatchIgnore(() => client.DownloadFile(Path.Combine(value, "favicon.ico").Replace('\\', '/'), file));
            }
        }

        private void DownloadFileAlternative(string value, string file)
        {
            HtmlWeb web = new HtmlWeb();
            HtmlDocument doc = ExceptionUtilities.TryAssignCatchIgnore(delegate { return web.Load(value); }, null) ;

            if (doc == null)
                return;

            foreach (var node in doc.DocumentNode.Descendants("link"))
            {
                var relValue = node.GetAttributeValue("rel", null);
                if (relValue?.ToLower() == "shortcut icon")
                {
                    using (WebClient client = new WebClient())
                    {
                        ExceptionUtilities.TryCatchIgnore(() => client.DownloadFile(node.GetAttributeValue("href", null), file));
                    }
                    continue;
                }

                var typeValue = node.GetAttributeValue("image/x-icon", null);
                if(typeValue != null)
                {
                    using (WebClient client = new WebClient())
                    {
                        ExceptionUtilities.TryCatchIgnore(() => client.DownloadFile(node.GetAttributeValue("type", null), file));
                    }
                }
            }
        }

        #endregion

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion
    }
}

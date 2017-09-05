using HtmlAgilityPack;
using System;
using System.Globalization;
using System.IO;
using System.Net;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace PasswordManager
{
    class ExtraDataToImageConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if(targetType == typeof(ImageSource))
            {
                BitmapImage returnVal = GetImage(value as string);
                if(returnVal == null)
                {
                    returnVal = GetImage(value as string, false);
                    if(returnVal != null)
                    {
                        return returnVal;
                    }
                }
                else
                {
                    return returnVal;
                }
                return new BitmapImage(new Uri("/PasswordManager;component/Resources/shield.ico", UriKind.RelativeOrAbsolute));
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        private void DownloadFile(string value, string file, bool favIcon)
        {
            if (favIcon)
            {
                using (WebClient client = new WebClient())
                {
                    client.DownloadFile(Path.Combine(value, "favicon.ico").Replace('\\', '/'), file);
                }
            }
            else
            {
                DownloadFileAlternative(value, file);
            }
        }

        private void DownloadFileAlternative(string value, string file)
        {
            var web = new HtmlWeb();
            var doc = web.Load(value);
            foreach (var node in doc.DocumentNode.Descendants("link"))
            {
                var relValue = node.GetAttributeValue("rel", null);
                if (relValue?.ToLower() == "shortcut icon")
                {
                    using (WebClient client = new WebClient())
                    {
                        client.DownloadFile(node.GetAttributeValue("href", null), file);
                    }
                }
            }
        }

        private BitmapImage GetImage(string value, bool favIcon = true)
        {
            var uri = new Uri(value);
            value = uri.Scheme + "://" + uri.Host;

            string file = Path.Combine(Path.GetTempPath(), "soteriapass", value.Replace(':', '_').Replace('.', '_').Replace('/', '_'), "favicon.ico");
            try
            {
                if (!Directory.Exists(Path.GetDirectoryName(file)))
                {
                    Directory.CreateDirectory(Path.GetDirectoryName(file));
                }
                if (!favIcon)
                {
                    file = Path.ChangeExtension(file, ".sec.ico");
                    if (!File.Exists(file))
                    {
                        DownloadFile(value, file, favIcon);
                    }
                }
                else if (!File.Exists(file))
                {
                    DownloadFile(value, file, favIcon);
                }
                return new BitmapImage(new Uri(file, UriKind.RelativeOrAbsolute));
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine(ex.Message);
            }
            return null;
        }

        #endregion
    }
}

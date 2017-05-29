using System;
using System.Collections.Generic;
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
                using (WebClient client = new WebClient())
                {
                    try
                    {
                        string file = Path.Combine(Path.GetTempPath(), (value as string).Replace(':', '_').Replace('.', '_').Replace('/', '_'), "favicon.ico");
                        if(!Directory.Exists(Path.GetDirectoryName(file)))
                        {
                            Directory.CreateDirectory(Path.GetDirectoryName(file));
                        }
                        if (!File.Exists(file))
                        {
                            client.DownloadFile(Path.Combine(value as string, "favicon.ico").Replace('\\', '/'), file);
                        }
                        return new BitmapImage(new Uri(file, UriKind.RelativeOrAbsolute));
                    }
                    catch
                    {

                    }
                }
                return new BitmapImage(new Uri("/PasswordManager;component/Resources/shield.ico", UriKind.RelativeOrAbsolute));
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}

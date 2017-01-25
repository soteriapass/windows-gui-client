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
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if(targetType == typeof(ImageSource))
            {
                string tempFile = Path.GetTempFileName();
                using (WebClient client = new WebClient())
                {
                    try
                    {
                        client.DownloadFile(Path.Combine(value as string, "favicon.ico").Replace('\\','/'), tempFile);
                        return new BitmapImage(new Uri(tempFile, UriKind.RelativeOrAbsolute));
                    }
                    catch
                    {

                    }
                }
                return new BitmapImage(new Uri("/PasswordManager;component/Resources/locked.ico", UriKind.RelativeOrAbsolute));
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}

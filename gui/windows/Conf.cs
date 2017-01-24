using Newtonsoft.Json;
using System.ComponentModel;
using System.IO;

namespace PasswordManager
{
    [JsonObject(MemberSerialization.OptIn)]
    class Conf
    {
        #region Variables

        [JsonProperty]
        private string _Username;

        [JsonProperty]
        private string _ServerCertificate;

        [JsonProperty]
        private string _AuthenticationAddress;

        [JsonProperty]
        private string _AuthenticationPort;

        [JsonProperty]
        private string _Password;

        #endregion

        #region Ctor

        ~Conf()
        {
            Save("pswmgr.conf");
        }

        #endregion

        #region Properties

        [Browsable(false)]
        public string Username
        {
            get { return _Username; }
            set { _Username = value; }
        }

        public string ServerCertificate
        {
            get { return _ServerCertificate; }
            set { _ServerCertificate = value; }
        }

        public string AuthenticationAddress
        {
            get { return _AuthenticationAddress; }
            set { _AuthenticationAddress = value; }
        }

        public string AuthenticationPort
        {
            get { return _AuthenticationPort; }
            set { _AuthenticationPort = value; }
        }
        
        [Browsable(false)]
        public string AuthenticationChannel
        {
            get { return string.Format("{0}:{1}", _AuthenticationAddress, _AuthenticationPort); }
        }

        [Browsable(false)]
        public string Password
        {
            get { return _Password; }
            set { _Password = value; }
        }

        #endregion

        #region Methods

        public static Conf Load(string filename)
        {
            if (!File.Exists(filename))
                return new Conf();

            string fileData = File.ReadAllText(filename);
            return JsonConvert.DeserializeObject<Conf>(fileData);
        }

        public void Save(string filename)
        {
            File.WriteAllText(filename, JsonConvert.SerializeObject(this, Formatting.Indented));
        }

        #endregion
    }
}

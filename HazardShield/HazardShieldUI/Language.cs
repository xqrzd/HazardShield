/*
*  Copyright (C) 2015 Orbitech
*
*  Authors: xqrzd
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*  MA 02110-1301, USA.
*/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HazardShieldUI
{
    class Language
    {
        /// <summary>
        /// Map of key to text (eg. MY_TEXT_VALUE -> MyValue).
        /// </summary>
        Dictionary<string, string> Text;

        public Language()
        {
            Text = new Dictionary<string, string>();
        }

        /// <summary>
        /// Loads a language file into memory. Whitespace is stripped from keys.
        /// </summary>
        /// <param name="languageFilePath">The full path to the language file.</param>
        /// <returns>True if the file was loaded.</returns>
        public bool LoadLanguage(string languageFilePath)
        {
            if (!File.Exists(languageFilePath))
                return false;

            Text.Clear();

            using (StreamReader reader = new StreamReader(languageFilePath))
            {
                string line;
                while ((line = reader.ReadLine()) != null)
                {
                    if (line.Length > 5 && line[0] != '\\' && line.Contains('='))
                    {
                        string[] parts = line.Split('=');
                        Text.Add(parts[0].Trim(), parts[1].Replace(@"\n", Environment.NewLine));
                    }
                }
            }

            return true;
        }

        /// <summary>
        /// Gets localized text for a given key from the currently loaded language.
        /// </summary>
        /// <param name="key">The key, as it appears in the language file.</param>
        /// <param name="returnEmptyIfNotFound">If this is set, an empty string will be returned if the key isn't found. Otherwise, the key will be returned.</param>
        /// <returns>Localized text.</returns>
        public string GetText(string key, bool returnEmptyIfNotFound = false)
        {
            string text;
            if (Text.TryGetValue(key, out text))
                return text;
            else
                return returnEmptyIfNotFound ? string.Empty : key;
        }

        /// <summary>
        /// Returns all language files in a directory, eg. English.
        /// </summary>
        /// <param name="languageDirectory">The directory to search.</param>
        /// <returns>An array of found languages.</returns>
        public static string[] GetLanguages(string languageDirectory)
        {
            string[] languages = Directory.GetFiles(languageDirectory, "*.txt", SearchOption.TopDirectoryOnly);

            for (int i = 0; i < languages.Length; i++)
                languages[i] = Path.GetFileNameWithoutExtension(languages[i]);

            return languages;
        }
    }
}
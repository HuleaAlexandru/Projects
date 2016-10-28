package ro.pub.cs.systems.eim.lab07.xkcdcartoondisplayer.view;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.ImageRequest;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.ResponseHandler;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.BasicResponseHandler;
import org.apache.http.impl.client.DefaultHttpClient;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;

import java.io.IOException;

import ro.pub.cs.systems.eim.lab07.xkcdcartoondisplayer.R;
import ro.pub.cs.systems.eim.lab07.xkcdcartoondisplayer.entities.XKCDCartoonInformation;
import ro.pub.cs.systems.eim.lab07.xkcdcartoondisplayer.general.Constants;
import ro.pub.cs.systems.eim.lab07.xkcdcartoondisplayer.network.VolleyController;

public class XKCDCartoonDisplayerActivity extends AppCompatActivity {

    private TextView xkcdCartoonTitleTextView;
    private ImageView xkcdCartoonImageView;
    private TextView xkcdCartoonUrlTextView;
    private Button previousButton, nextButton;

    private class XKCDCartoonUrlButtonClickListener implements Button.OnClickListener {

        String xkcdComicUrl;

        public XKCDCartoonUrlButtonClickListener(String xkcdComicUrl) {
            this.xkcdComicUrl = xkcdComicUrl;
        }

        @Override
        public void onClick(View view) {
            new XKCDCartoonDisplayerAsyncTask().execute(xkcdComicUrl);
        }
    }

    private class XKCDCartoonDisplayerAsyncTask extends AsyncTask<String, Void, XKCDCartoonInformation> {

        @Override
        protected XKCDCartoonInformation doInBackground(String... urls) {
            XKCDCartoonInformation xkcdCartoonInformation = new XKCDCartoonInformation();
            HttpClient httpClient = new DefaultHttpClient();
            HttpGet httpXkcdGet = new HttpGet(urls[0]);
            ResponseHandler<String> responseHandlerGet = new BasicResponseHandler();
            String pageSourceCode = null;
            try {
                pageSourceCode = httpClient.execute(httpXkcdGet, responseHandlerGet);
            } catch (ClientProtocolException clientProtocolException) {
                Log.e(Constants.TAG, clientProtocolException.getMessage());
                if (Constants.DEBUG) {
                    clientProtocolException.printStackTrace();
                }
            } catch (IOException ioException) {
                Log.e(Constants.TAG, ioException.getMessage());
                if (Constants.DEBUG) {
                    ioException.printStackTrace();
                }
            }
            if (pageSourceCode != null) {
                Document document = Jsoup.parse(pageSourceCode);
                Element htmlTag = document.child(0);

                // cartoon title
                Element divTagIdCtitle = htmlTag.getElementsByAttributeValue(Constants.ID_ATTRIBUTE, Constants.CTITLE_VALUE).first();
                xkcdCartoonInformation.setCartoonTitle(divTagIdCtitle.ownText());

                // cartoon content
                Element divTagIdComic = htmlTag.getElementsByAttributeValue(Constants.ID_ATTRIBUTE, Constants.COMIC_VALUE).first();
                String cartoonInternetAddress = divTagIdComic.getElementsByTag(Constants.IMG_TAG).attr(Constants.SRC_ATTRIBUTE);
                String cartoonUrl = Constants.HTTP_PROTOCOL + cartoonInternetAddress;
                xkcdCartoonInformation.setCartoonUrl(cartoonUrl);

                // cartoon links urls
                Element aTagRelPrev = htmlTag.getElementsByAttributeValue(Constants.REL_ATTRIBUTE, Constants.PREVIOUS_VALUE).first();
                String previousCartoonInternetAddress = Constants.XKCD_INTERNET_ADDRESS + aTagRelPrev.attr(Constants.HREF_ATTRIBUTE);
                xkcdCartoonInformation.setPreviousCartoonUrl(previousCartoonInternetAddress);

                Element aTagRelNext = htmlTag.getElementsByAttributeValue(Constants.REL_ATTRIBUTE, Constants.NEXT_VALUE).first();
                String nextCartoonInternetAddress = Constants.XKCD_INTERNET_ADDRESS + aTagRelNext.attr(Constants.HREF_ATTRIBUTE);
                xkcdCartoonInformation.setNextCartoonUrl(nextCartoonInternetAddress);
            }

            return xkcdCartoonInformation;
        }

        @Override
        protected void onPostExecute(XKCDCartoonInformation xkcdCartoonInformation) {
            if (xkcdCartoonInformation != null) {
                String cartoonTitle = xkcdCartoonInformation.getCartoonTitle();
                if (cartoonTitle != null) {
                    xkcdCartoonTitleTextView.setText(cartoonTitle);
                }
                String cartoonUrl = xkcdCartoonInformation.getCartoonUrl();
                if (cartoonUrl != null) {
                    xkcdCartoonUrlTextView.setText(cartoonUrl);
                    ImageRequest cartoonRequest = new ImageRequest(
                            cartoonUrl,
                            new Response.Listener<Bitmap>() {
                                @Override
                                public void onResponse(Bitmap bitmap) {
                                    xkcdCartoonImageView.setImageBitmap(bitmap);
                                }
                            },
                            0,
                            0,
                            null,
                            Bitmap.Config.RGB_565,
                            new Response.ErrorListener() {
                                public void onErrorResponse(VolleyError volleyError) {
                                    Log.d(Constants.TAG, volleyError.toString());
                                    if (Constants.DEBUG) {
                                        Toast.makeText(xkcdCartoonTitleTextView.getContext(), xkcdCartoonTitleTextView.getResources().getString(R.string.an_error_has_occurred), Toast.LENGTH_LONG)
                                                .show();
                                    }
                                }
                            }
                    );
                    VolleyController.getInstance(xkcdCartoonImageView.getContext()).addToRequestQueue(cartoonRequest);
                }
                String previousCartoonUrl = xkcdCartoonInformation.getPreviousCartoonUrl();
                if (previousCartoonUrl != null) {
                    previousButton.setOnClickListener(new XKCDCartoonUrlButtonClickListener(previousCartoonUrl));
                }
                String nextCartoonUrl = xkcdCartoonInformation.getNextCartoonUrl();
                if (nextCartoonUrl != null) {
                    nextButton.setOnClickListener(new XKCDCartoonUrlButtonClickListener(nextCartoonUrl));
                }
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_xkcd_cartoon_displayer);

        xkcdCartoonTitleTextView = (TextView)findViewById(R.id.xkcd_cartoon_title_text_view);
        xkcdCartoonImageView = (ImageView)findViewById(R.id.xkcd_cartoon_image_view);
        xkcdCartoonUrlTextView = (TextView)findViewById(R.id.xkcd_cartoon_url_text_view);

        previousButton = (Button)findViewById(R.id.previous_button);
        nextButton = (Button)findViewById(R.id.next_button);

        new XKCDCartoonDisplayerAsyncTask().execute(Constants.XKCD_INTERNET_ADDRESS);
    }
}

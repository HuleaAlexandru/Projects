package ro.pub.cs.systems.eim.lab07.xkcdcartoondisplayer.entities;

public class XKCDCartoonInformation {

    private String cartoonTitle;
    private String cartoonUrl;
    private String previousCartoonUrl, nextCartoonUrl;

    public XKCDCartoonInformation() {
        this.cartoonTitle = new String();
        this.cartoonUrl = new String();
        this.previousCartoonUrl = new String();
        this.nextCartoonUrl = new String();
    }

    public XKCDCartoonInformation(String cartoonTitle,
                                  String cartoonUrl,
                                  String previousCartoonUrl,
                                  String nextCartoonUrl) {
        this.cartoonTitle = cartoonTitle;
        this.cartoonUrl = cartoonUrl;
        this.previousCartoonUrl = previousCartoonUrl;
        this.nextCartoonUrl = nextCartoonUrl;
    }

    public void setCartoonTitle(String cartoonTitle) {
        this.cartoonTitle = cartoonTitle;
    }

    public String getCartoonTitle() {
        return cartoonTitle;
    }

    public void setCartoonUrl(String cartoonUrl) {
        this.cartoonUrl = cartoonUrl;
    }

    public String getCartoonUrl() {
        return cartoonUrl;
    }

    public void setPreviousCartoonUrl(String previousCartoonUrl) {
        this.previousCartoonUrl = previousCartoonUrl;
    }

    public String getPreviousCartoonUrl() {
        return previousCartoonUrl;
    }

    public void setNextCartoonUrl(String nextCartoonUrl) {
        this.nextCartoonUrl = nextCartoonUrl;
    }

    public String getNextCartoonUrl() {
        return nextCartoonUrl;
    }

}
